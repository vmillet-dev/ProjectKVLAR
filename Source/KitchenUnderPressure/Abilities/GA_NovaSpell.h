// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_Spell.h"
#include "GA_NovaSpell.generated.h"

/**
 * Nova form: an instant 360° burst around the caster. Server-authoritative overlap that applies the
 * spell's damage and status to every ability system in range.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UGA_NovaSpell : public UGameplayAbility_Spell
{
	GENERATED_BODY()

public:
	UGA_NovaSpell();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Burst radius in cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float Radius = 400.f;
};
