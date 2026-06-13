// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "ElementDefinition.generated.h"

class UGameplayEffect;

/**
 * Spell layer 1 (Element): the damage type, its colour and its on-hit status effect. Authored as a
 * data asset and resolved at runtime from FSpellDefinition.Element via USpellRegistrySubsystem.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UElementDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Identity tag (KUPTags::Element_Fire, ...). Must match the tag stored in generated spells. */
	UPROPERTY(EditAnywhere, Category = "Element")
	FGameplayTag ElementTag;

	/** Colour driving every visual for this element (projectile, nova, combo, hand aura). */
	UPROPERTY(EditAnywhere, Category = "Element")
	FLinearColor Color = FLinearColor::White;

	/** Base damage before the form multiplier and the spell's Power are applied. */
	UPROPERTY(EditAnywhere, Category = "Element")
	float BaseDamage = 25.f;

	/** Status GE applied on hit (burn / freeze / poison). Optional. */
	UPROPERTY(EditAnywhere, Category = "Element")
	TSubclassOf<UGameplayEffect> StatusEffect;
};
