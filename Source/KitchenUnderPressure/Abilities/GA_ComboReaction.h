// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_Spell.h"
#include "GA_ComboReaction.generated.h"

class AComboBurst;

/**
 * The two-hand alchemical reaction. Triggered by the caster when both hands fire within the combo
 * window; reads the pre-computed combo (location, blended colour, damage, status) from the caster,
 * applies an area burst to every ability system in range, and spawns the visual. Server-authoritative.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UGA_ComboReaction : public UGameplayAbility_Spell
{
	GENERATED_BODY()

public:
	UGA_ComboReaction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Visual spawned at the reaction point (defaults to the built-in AComboBurst). */
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TSubclassOf<AComboBurst> BurstClass;
};
