// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellCasterComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbility_Spell.h"
#include "Player/AlchemistPlayerState.h"
#include "Spells/SpellConfig.h"
#include "Spells/SpellRegistrySubsystem.h"
#include "Spells/ElementDefinition.h"
#include "Spells/FormDefinition.h"
#include "Spells/ModifierDefinition.h"
#include "Spells/SpellGenerator.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

USpellCasterComponent::USpellCasterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// No replicated properties (the loadout lives on the PlayerState), but the component must still
	// replicate so the owning client can route its Server RPC through the actor channel.
	SetIsReplicatedByDefault(true);
}

UAbilitySystemComponent* USpellCasterComponent::GetASC() const
{
	// Resolves through the owner's IAbilitySystemInterface to the PlayerState ability system.
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

AAlchemistPlayerState* USpellCasterComponent::GetAlchemistPlayerState() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Pawn->GetPlayerState<AAlchemistPlayerState>() : nullptr;
}

const FSpellDefinition& USpellCasterComponent::GetSpell(EHand Hand) const
{
	static const FSpellDefinition EmptySpell;
	const AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	return PS ? PS->GetSpell(Hand) : EmptySpell;
}

bool USpellCasterComponent::GetHandForHandle(FGameplayAbilitySpecHandle Handle, EHand& OutHand) const
{
	const AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	if (!PS || !Handle.IsValid())
	{
		return false;
	}
	if (Handle == PS->LeftSpellHandle)
	{
		OutHand = EHand::Left;
		return true;
	}
	if (Handle == PS->RightSpellHandle)
	{
		OutHand = EHand::Right;
		return true;
	}
	return false;
}

bool USpellCasterComponent::GetSpellForHandle(FGameplayAbilitySpecHandle Handle, FSpellDefinition& OutSpell) const
{
	const AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	if (!PS)
	{
		return false;
	}
	if (Handle == PS->LeftSpellHandle)
	{
		OutSpell = PS->GetSpell(EHand::Left);
		return OutSpell.IsValid();
	}
	if (Handle == PS->RightSpellHandle)
	{
		OutSpell = PS->GetSpell(EHand::Right);
		return OutSpell.IsValid();
	}
	return false;
}

void USpellCasterComponent::ServerInitializeAbilities()
{
	if (bAbilitiesGranted || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	UAbilitySystemComponent* ASC = GetASC();
	if (!PS || !ASC)
	{
		return;
	}
	bAbilitiesGranted = true;

	if (!PS->HasRolledStartingSpells())
	{
		// Fresh run: one independent spell per hand. Only the server generates; the results replicate
		// from the PlayerState as FSpellDefinition, so all clients agree without re-rolling.
		PS->MarkStartingSpellsRolled();
		USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
		FRandomStream Rng(FMath::Rand());
		EquipSpell(EHand::Left, USpellGenerator::GenerateRandomSpell(Registry, Rng));
		EquipSpell(EHand::Right, USpellGenerator::GenerateRandomSpell(Registry, Rng));
	}
	else
	{
		// Respawn / travel: keep the persisted loadout, just re-grant its abilities for this new pawn
		// (EquipSpell clears the previous pawn's grants from the persistent ASC first).
		EquipSpell(EHand::Left, PS->GetSpell(EHand::Left));
		EquipSpell(EHand::Right, PS->GetSpell(EHand::Right));
	}

	if (ComboAbility)
	{
		if (PS->ComboHandle.IsValid())
		{
			ASC->ClearAbility(PS->ComboHandle);
		}
		PS->ComboHandle = ASC->GiveAbility(FGameplayAbilitySpec(ComboAbility, 1, INDEX_NONE, this));
	}
}

void USpellCasterComponent::EquipSpell(EHand Hand, const FSpellDefinition& Spell)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	UAbilitySystemComponent* ASC = GetASC();
	if (!PS || !ASC)
	{
		return;
	}

	// Resolve the form to its ability class (fall back to the safety-net ability if it can't be found).
	TSubclassOf<UGameplayAbility> AbilityClass = FallbackSpellAbility;
	if (USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this))
	{
		if (const UFormDefinition* Form = Registry->FindForm(Spell.Form))
		{
			if (Form->AbilityClass)
			{
				AbilityClass = Form->AbilityClass;
			}
		}
	}

	// Clear any previous ability granted to this hand (possibly by a previous pawn's component).
	FGameplayAbilitySpecHandle& Handle = (Hand == EHand::Left) ? PS->LeftSpellHandle : PS->RightSpellHandle;
	if (Handle.IsValid())
	{
		ASC->ClearAbility(Handle);
		Handle = FGameplayAbilitySpecHandle();
	}

	// Store the definition first so the granted ability can read it back on activation.
	PS->SetSpell(Hand, Spell);

	if (AbilityClass)
	{
		Handle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
	}
}

void USpellCasterComponent::TryCast(EHand Hand)
{
	ServerActivateSpell(Hand);
}

void USpellCasterComponent::ServerActivateSpell_Implementation(EHand Hand)
{
	UAbilitySystemComponent* ASC = GetASC();
	const AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	if (!ASC || !PS)
	{
		return;
	}

	const FGameplayAbilitySpecHandle Handle = (Hand == EHand::Left) ? PS->LeftSpellHandle : PS->RightSpellHandle;

	// Only a cast that actually went through counts toward a combo — a hand on cooldown (or whose
	// activation failed for any reason) must not trigger a free reaction.
	if (Handle.IsValid() && ASC->TryActivateAbility(Handle))
	{
		RecordCastAndTryCombo(Hand);
	}
}

void USpellCasterComponent::RecordCastAndTryCombo(EHand Hand)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FSpellDefinition& Spell = GetSpell(Hand);
	const float Now = World->GetTimeSeconds();

	FHandCastRecord& Record = (Hand == EHand::Left) ? LastLeftCast : LastRightCast;
	Record.Element = Spell.Element;
	Record.Modifiers = Spell.Modifiers;
	Record.Time = Now;

	USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
	const USpellConfig* Config = Registry ? Registry->GetConfig() : nullptr;
	const float Window = Config ? Config->ComboWindow : 0.35f;

	if (LastLeftCast.Element.IsValid() && LastRightCast.Element.IsValid()
		&& (Now - LastLeftCast.Time) <= Window && (Now - LastRightCast.Time) <= Window)
	{
		TriggerCombo();

		// Debounce so the same pair can't immediately fire a second combo.
		LastLeftCast.Time = -1000.f;
		LastRightCast.Time = -1000.f;
	}
}

void USpellCasterComponent::TriggerCombo()
{
	UAbilitySystemComponent* ASC = GetASC();
	const AAlchemistPlayerState* PS = GetAlchemistPlayerState();
	USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
	const USpellConfig* Config = Registry ? Registry->GetConfig() : nullptr;
	if (!ASC || !PS || !PS->ComboHandle.IsValid() || !Config)
	{
		return;
	}

	const FGameplayTag LeftElement = LastLeftCast.Element;
	const FGameplayTag RightElement = LastRightCast.Element;

	const UElementDefinition* LeftDef = Registry->FindElement(LeftElement);
	const UElementDefinition* RightDef = Registry->FindElement(RightElement);
	const FLinearColor LeftColor = LeftDef ? LeftDef->Color : FLinearColor::White;
	const FLinearColor RightColor = RightDef ? RightDef->Color : FLinearColor::White;

	float Multiplier;
	FLinearColor BlendedColor;
	if (LeftElement == RightElement)
	{
		// Amplification: identical elements double the damage and fuse the (identical) colour.
		Multiplier = Config->SameElementComboMultiplier;
		BlendedColor = LeftColor;
	}
	else if (Config->AreElementsOpposed(LeftElement, RightElement))
	{
		// Reaction: opposed elements explode harder; add the colours for a brighter blend.
		Multiplier = Config->OpposedComboMultiplier;
		BlendedColor = LeftColor + RightColor;
	}
	else
	{
		// Elements neither match nor oppose: no reaction.
		return;
	}

	// GDD 5.2: a modifier present on BOTH hands applies to the reaction itself.
	const FGameplayTagContainer SharedModifiers = LastLeftCast.Modifiers.FilterExact(LastRightCast.Modifiers);
	for (const FGameplayTag& ModifierTag : SharedModifiers)
	{
		if (const UModifierDefinition* Modifier = Registry->FindModifier(ModifierTag))
		{
			Multiplier *= Modifier->DamageScale;
		}
	}

	const float BaseDamage = (LeftDef ? LeftDef->BaseDamage : 25.f) + (RightDef ? RightDef->BaseDamage : 25.f);

	PendingCombo.Location = ComputeComboLocation();
	PendingCombo.Color = BlendedColor;
	PendingCombo.Damage = BaseDamage * Multiplier;
	PendingCombo.Radius = ComboRadius;
	PendingCombo.StatusEffect = LeftDef ? LeftDef->StatusEffect : (RightDef ? RightDef->StatusEffect : nullptr);

	ASC->TryActivateAbility(PS->ComboHandle);
}

FVector USpellCasterComponent::ComputeComboLocation() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const UWorld* World = GetWorld();
	if (!Pawn || !World)
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	Pawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	const FVector End = EyeLocation + EyeRotation.Vector() * 1500.f;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ComboTrace), /*bTraceComplex=*/false, GetOwner());
	if (World->LineTraceSingleByChannel(Hit, EyeLocation, End, ECC_Visibility, Params))
	{
		return Hit.ImpactPoint;
	}
	return End;
}
