// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayAbility_Spell.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "AbilitySystemComponent.h"
#include "Spells/SpellCasterComponent.h"
#include "Spells/SpellGenerationSettings.h"
#include "Spells/ElementDefinition.h"
#include "Spells/FormDefinition.h"
#include "Spells/ModifierDefinition.h"
#include "KUPGameplayTags.h"

UGameplayAbility_Spell::UGameplayAbility_Spell()
{
	// One instance per spec (= per hand); the caster routes a Server RPC and the ability runs on the
	// server only, spawning replicated actors/effects. This deliberately avoids GAS prediction for the
	// first slice.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UGameplayAbility_Spell::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const float Cooldown = GetEffectiveCooldownSeconds();
	if (Cooldown > 0.f)
	{
		if (const UWorld* World = GetWorld())
		{
			if (World->GetTimeSeconds() - LastCommitTime < Cooldown)
			{
				return false;
			}
		}
	}
	return true;
}

float UGameplayAbility_Spell::GetEffectiveCooldownSeconds() const
{
	FSpellDefinition Spell;
	if (TryGetEquippedSpell(Spell) && Spell.IsValid())
	{
		if (USpellGenerationSettings* Settings = GetMutableDefault<USpellGenerationSettings>())
		{
			if (const UFormDefinition* Form = Settings->FindForm(Spell.Form))
			{
				return Form->CooldownSeconds;
			}
		}
	}
	return CooldownSeconds;
}

bool UGameplayAbility_Spell::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (const UWorld* World = GetWorld())
	{
		LastCommitTime = World->GetTimeSeconds();
	}
	return true;
}

void UGameplayAbility_Spell::GetMuzzle(FVector& OutLocation, FVector& OutDirection) const
{
	OutLocation = FVector::ZeroVector;
	OutDirection = FVector::ForwardVector;

	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// Eyes view point uses the (replicated) control rotation, so server-side aim matches the client.
	FVector EyeLocation;
	FRotator EyeRotation;
	Avatar->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	const FVector ViewDirection = EyeRotation.Vector();

	OutLocation = EyeLocation;
	OutDirection = ViewDirection;

	// Hand-less abilities (e.g. the combo) keep the centred eye muzzle.
	EHand Hand;
	if (!TryGetHand(Hand))
	{
		return;
	}

	// Offset the muzzle to the casting hand's side, slightly below eye height.
	const FRotationMatrix ViewMatrix(EyeRotation);
	const float Side = (Hand == EHand::Left) ? -1.f : 1.f;
	OutLocation = EyeLocation
		+ ViewMatrix.GetUnitAxis(EAxis::Y) * (Side * MuzzleLateralOffset)
		+ ViewMatrix.GetUnitAxis(EAxis::Z) * MuzzleVerticalOffset;

	// Converge on the crosshair: aim at what the centre of the screen looks at, so an offset muzzle
	// still hits the aimed target instead of flying parallel to it.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SpellAimTrace), /*bTraceComplex=*/false, Avatar);
	const FVector TraceEnd = EyeLocation + ViewDirection * 10000.f;
	const FVector AimPoint = Avatar->GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, Params)
		? Hit.ImpactPoint
		: TraceEnd;

	OutDirection = (AimPoint - OutLocation).GetSafeNormal();
	if (OutDirection.IsNearlyZero())
	{
		OutDirection = ViewDirection;
	}
}

bool UGameplayAbility_Spell::TryGetEquippedSpell(FSpellDefinition& OutSpell) const
{
	if (const USpellCasterComponent* Caster = Cast<USpellCasterComponent>(GetCurrentSourceObject()))
	{
		return Caster->GetSpellForHandle(GetCurrentAbilitySpecHandle(), OutSpell);
	}
	return false;
}

bool UGameplayAbility_Spell::TryGetHand(EHand& OutHand) const
{
	if (const USpellCasterComponent* Caster = Cast<USpellCasterComponent>(GetCurrentSourceObject()))
	{
		return Caster->GetHandForHandle(GetCurrentAbilitySpecHandle(), OutHand);
	}
	return false;
}

FGameplayTag UGameplayAbility_Spell::GetEquippedElement() const
{
	FSpellDefinition Spell;
	return TryGetEquippedSpell(Spell) ? Spell.Element : FGameplayTag();
}

void UGameplayAbility_Spell::ResolvePayload(float& OutDamage, FLinearColor& OutColor, TSubclassOf<UGameplayEffect>& OutStatusEffect, float& OutVisualScale) const
{
	// Defaults, used if no spell is equipped (e.g. a purely Blueprint-configured ability).
	OutDamage = Damage;
	OutColor = ElementColor;
	OutStatusEffect = nullptr;
	OutVisualScale = 1.f;

	FSpellDefinition Spell;
	if (!TryGetEquippedSpell(Spell) || !Spell.IsValid())
	{
		return;
	}

	USpellGenerationSettings* Settings = GetMutableDefault<USpellGenerationSettings>();
	if (!Settings)
	{
		return;
	}

	const UElementDefinition* Element = Settings->FindElement(Spell.Element);
	const UFormDefinition* Form = Settings->FindForm(Spell.Form);

	const float Base = Element ? Element->BaseDamage : Damage;
	const float FormMultiplier = Form ? Form->DamageMultiplier : 1.f;
	OutDamage = Base * FormMultiplier * Spell.Power;
	OutColor = Element ? Element->Color : ElementColor;
	OutStatusEffect = Element ? Element->StatusEffect : nullptr;

	// Layer 3 (modifiers): each one scales the damage — this is what pushes rare/epic spells past the
	// Power curve alone ("1-2 paramètres poussés à l'extrême").
	for (const FGameplayTag& ModifierTag : Spell.Modifiers)
	{
		if (const UModifierDefinition* Modifier = Settings->FindModifier(ModifierTag))
		{
			OutDamage *= Modifier->DamageScale;
		}
	}

	// "Strong parameter = strong visual": the visual grows with power (which itself encodes rarity).
	OutVisualScale = FMath::Clamp(Spell.Power, 0.5f, 4.f);
}

TSubclassOf<UGameplayEffect> UGameplayAbility_Spell::GetDamageEffectClass() const
{
	if (const USpellGenerationSettings* Settings = GetDefault<USpellGenerationSettings>())
	{
		return Settings->DamageEffect;
	}
	return nullptr;
}

void UGameplayAbility_Spell::ApplyDamageAndStatus(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InDamage, TSubclassOf<UGameplayEffect> StatusEffect) const
{
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	if (const TSubclassOf<UGameplayEffect> DamageGE = GetDamageEffectClass())
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageGE, 1.f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(KUPTags::Data_Damage, InDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
		}
	}

	if (StatusEffect)
	{
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(StatusEffect, 1.f, Context);
		if (Spec.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
		}
	}
}
