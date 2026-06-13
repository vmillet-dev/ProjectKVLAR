// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ModifierDefinition.generated.h"

/**
 * Spell layer 3 (Modifier): an extra behaviour that alters a spell. Absent on common spells, present
 * (and sometimes multiple) on rarer ones. The first slice only carries the identity tag and a damage
 * knob; richer behaviours (bounce, pierce, multi-projectile) hang off the tag later.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UModifierDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Identity tag (KUPTags::Modifier_*). */
	UPROPERTY(EditAnywhere, Category = "Modifier")
	FGameplayTag ModifierTag;

	/** Damage multiplier applied while this modifier is present (placeholder knob for the slice). */
	UPROPERTY(EditAnywhere, Category = "Modifier")
	float DamageScale = 1.f;
};
