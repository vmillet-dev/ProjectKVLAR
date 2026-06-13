// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlchemistAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UAlchemistAttributeSet::UAlchemistAttributeSet()
{
	// Placeholder defaults so a freshly spawned actor is never at 0 HP before its init GameplayEffect
	// runs. Real starting values come from GE_PlayerInit / GE_EnemyInit applied on the server.
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMoveSpeed(600.f);
}

void UAlchemistAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always so the UI updates even when a value replicates back to the same number
	// (e.g. healed back to full between two updates).
	DOREPLIFETIME_CONDITION_NOTIFY(UAlchemistAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAlchemistAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAlchemistAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	// Damage is intentionally not replicated: it is a transient server-side accumulator.
}

void UAlchemistAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UAlchemistAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Consume the inbound damage meta-attribute and apply it to Health.
		const float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Direct Health sets (healing, init) still get clamped.
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}

	if (GetHealth() <= 0.f)
	{
		// Server-authoritative death signal; the owner runs the real logic and is responsible for
		// being idempotent (further damage can re-enter here while already dead).
		if (const UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			OnOutOfHealth.Broadcast(ASC->GetAvatarActor());
		}
	}
}

void UAlchemistAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAlchemistAttributeSet, Health, OldValue);
}

void UAlchemistAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAlchemistAttributeSet, MaxHealth, OldValue);
}

void UAlchemistAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAlchemistAttributeSet, MoveSpeed, OldValue);
}
