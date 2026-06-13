// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "Abilities/AlchemistAbilitySystemComponent.h"
#include "Attributes/AlchemistAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "System/KUPCombatStatics.h"
#include "KUPGameplayTags.h"
#include "Spells/SpellConfig.h"
#include "Spells/SpellRegistrySubsystem.h"
#include "Spells/ElementDefinition.h"
#include "Spells/ComboBurst.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

AEnemyCharacter::AEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAlchemistAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAlchemistAttributeSet>(TEXT("AttributeSet"));

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent)
	{
		return;
	}

	// AI both owns and is the avatar of its ability system.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		if (DefaultAttributesEffect)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			Context.AddSourceObject(this);
			const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}

		if (AttributeSet)
		{
			AttributeSet->OnOutOfHealth.AddUObject(this, &AEnemyCharacter::HandleOutOfHealth);
		}
	}
}

void AEnemyCharacter::ApplyHealthScale(float Multiplier)
{
	if (!HasAuthority() || !AttributeSet || Multiplier <= 0.f)
	{
		return;
	}

	// Raise the ceiling first: PreAttributeChange clamps Health to MaxHealth.
	AttributeSet->SetMaxHealth(AttributeSet->GetMaxHealth() * Multiplier);
	AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
}

void AEnemyCharacter::TryMeleeAttack(AActor* Target)
{
	if (!HasAuthority() || bDead || !Target || !MeleeEffect || !AbilitySystemComponent)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastMeleeTime < MeleeCooldown)
	{
		return;
	}
	if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > MeleeRange)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return;
	}

	LastMeleeTime = Now;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddInstigator(this, this);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(MeleeEffect, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(KUPTags::Data_Damage, MeleeDamage);
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}

void AEnemyCharacter::NotifyElementalHit(UAbilitySystemComponent* SourceASC, const FGameplayTag& Element)
{
	UWorld* World = GetWorld();
	USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
	const USpellConfig* Config = Registry ? Registry->GetConfig() : nullptr;
	if (!HasAuthority() || bDead || !SourceASC || !Element.IsValid() || !World || !Config)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float Window = Config->CrossPlayerComboWindow;

	// A second player landing the opposed element within the window completes the reaction. Both
	// hands of one player share an ASC, so the "different source" check leaves the solo two-hand
	// combo to the caster component.
	for (const FElementalHitRecord& Record : RecentElementalHits)
	{
		if ((Now - Record.Time) <= Window
			&& Record.SourceASC.IsValid() && Record.SourceASC.Get() != SourceASC
			&& Config->AreElementsOpposed(Record.Element, Element))
		{
			TriggerCrossPlayerCombo(Record, SourceASC, Element);
			RecentElementalHits.Reset(); // the pair is consumed
			return;
		}
	}

	RecentElementalHits.RemoveAll([Now, Window](const FElementalHitRecord& Record)
	{
		return (Now - Record.Time) > Window;
	});

	FElementalHitRecord& NewRecord = RecentElementalHits.AddDefaulted_GetRef();
	NewRecord.SourceASC = SourceASC;
	NewRecord.Element = Element;
	NewRecord.Time = Now;
}

void AEnemyCharacter::TriggerCrossPlayerCombo(const FElementalHitRecord& FirstHit, UAbilitySystemComponent* SecondASC, const FGameplayTag& SecondElement)
{
	UWorld* World = GetWorld();
	USpellRegistrySubsystem* Registry = USpellRegistrySubsystem::Get(this);
	const USpellConfig* Config = Registry ? Registry->GetConfig() : nullptr;
	if (!World || !Config)
	{
		return;
	}

	const UElementDefinition* FirstDef = Registry->FindElement(FirstHit.Element);
	const UElementDefinition* SecondDef = Registry->FindElement(SecondElement);

	// Same recipe as the solo opposed reaction: summed base damage, boosted, brighter blended colour.
	const float BaseDamage = (FirstDef ? FirstDef->BaseDamage : 25.f) + (SecondDef ? SecondDef->BaseDamage : 25.f);
	const float ReactionDamage = BaseDamage * Config->OpposedComboMultiplier;
	const FLinearColor Color = (FirstDef ? FirstDef->Color : FLinearColor::White) + (SecondDef ? SecondDef->Color : FLinearColor::White);

	// The reaction's plain damage GE carries no element, so it can never re-enter NotifyElementalHit.
	// Attributed to the player who completed the pair.
	UKUPCombatStatics::ApplyAreaDamage(World, SecondASC, GetActorLocation(), Config->CrossPlayerComboRadius, ReactionDamage, Config->DamageEffect, nullptr);

	if (AComboBurst* Burst = World->SpawnActor<AComboBurst>(AComboBurst::StaticClass(), GetActorLocation(), FRotator::ZeroRotator))
	{
		Burst->InitBurst(Color, Config->CrossPlayerComboRadius);
	}
}

void AEnemyCharacter::HandleOutOfHealth(AActor* DeadAvatar)
{
	if (bDead)
	{
		return;
	}
	bDead = true;

	if (AbilitySystemComponent)
	{
		// TagOnly replication so clients can read the death state before the actor despawns.
		AbilitySystemComponent->AddLooseGameplayTag(KUPTags::State_Dead, 1, EGameplayTagReplicationState::TagOnly);
		AbilitySystemComponent->CancelAllAbilities();
	}

	OnEnemyDied.Broadcast(this);

	// No ragdoll/animation in the first slice: stop interacting, then disappear shortly after (leaving
	// time for the death tag / zero health to replicate to clients).
	SetActorEnableCollision(false);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
	SetLifeSpan(0.2f);
}
