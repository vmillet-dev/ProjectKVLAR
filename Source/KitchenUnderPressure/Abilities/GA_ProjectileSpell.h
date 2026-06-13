// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_Spell.h"
#include "GA_ProjectileSpell.generated.h"

class ASpellProjectile;

/**
 * Projectile form: spawns a replicated ASpellProjectile travelling along the aim, carrying this
 * ability's damage/colour/effect payload. Server-authoritative.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UGA_ProjectileSpell : public UGameplayAbility_Spell
{
	GENERATED_BODY()

public:
	UGA_ProjectileSpell();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Projectile actor to spawn (assign BP_SpellProjectile). */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Launch speed in cm/s. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float ProjectileSpeed = 2200.f;
};
