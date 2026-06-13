// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "KUPCombatStatics.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * Shared combat rules used by every damage application site (projectile, nova, combos, melee). The
 * single source of truth for the friendly-fire gate, so a new damage source can't forget it.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UKUPCombatStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** True if the ASC belongs to a player (its owner is a PlayerState) rather than an AI pawn. */
	static bool IsPlayerASC(const UAbilitySystemComponent* ASC);

	/** Friendly-fire gate: false only when both sides are players and the run has friendly fire off. */
	static bool CanDamage(const UAbilitySystemComponent* SourceASC, const UAbilitySystemComponent* TargetASC);

	/** Server: apply the damage GE (SetByCaller Data.Damage) and optional status GE once per ability
	 *  system overlapping the sphere, honouring the friendly-fire gate. */
	static void ApplyAreaDamage(UWorld* World, UAbilitySystemComponent* SourceASC, const FVector& Center, float Radius, float DamageAmount, TSubclassOf<UGameplayEffect> DamageEffect, TSubclassOf<UGameplayEffect> StatusEffect);
};
