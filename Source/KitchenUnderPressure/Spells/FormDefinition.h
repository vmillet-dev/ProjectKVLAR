// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "FormDefinition.generated.h"

class UGameplayAbility_Spell;

/**
 * Spell layer 2 (Form): how the spell is delivered. Maps a form tag to the GameplayAbility that
 * performs the cast (projectile, nova, ...). The caster grants this ability class when a spell of
 * this form is equipped.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UFormDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Identity tag (KUPTags::Form_Projectile, ...). */
	UPROPERTY(EditAnywhere, Category = "Form")
	FGameplayTag FormTag;

	/** Ability granted/activated for this form (assign BP_GA_ProjectileSpell, BP_GA_NovaSpell, ...). */
	UPROPERTY(EditAnywhere, Category = "Form")
	TSubclassOf<UGameplayAbility_Spell> AbilityClass;

	/** Damage scaling for this form (e.g. a nova spread over an area may hit softer than a projectile). */
	UPROPERTY(EditAnywhere, Category = "Form")
	float DamageMultiplier = 1.f;

	/** Fire rate: minimum seconds between two casts of this form (e.g. projectile 0.5, nova 1.5).
	 *  Overrides the ability's own CooldownSeconds whenever a generated spell is equipped. */
	UPROPERTY(EditAnywhere, Category = "Form")
	float CooldownSeconds = 0.5f;
};
