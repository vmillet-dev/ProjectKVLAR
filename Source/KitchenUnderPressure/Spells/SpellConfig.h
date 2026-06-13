// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "Spells/SpellTypes.h"
#include "SpellConfig.generated.h"

class UElementDefinition;
class UFormDefinition;
class UModifierDefinition;
class UGameplayEffect;

/** A pair of elements that react when combined two-handed (order-independent). */
USTRUCT()
struct FElementOpposition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combo")
	FGameplayTag ElementA;

	UPROPERTY(EditAnywhere, Category = "Combo")
	FGameplayTag ElementB;
};

/**
 * Authored spell content and tuning: the element/form/modifier pools, the rarity weights and power
 * curve, and the combo settings. Pure data + pure helpers. The runtime tag->definition lookup lives
 * in USpellRegistrySubsystem, which loads the pools from the active config; USpellSettings selects it.
 */
UCLASS(BlueprintType)
class KITCHENUNDERPRESSURE_API USpellConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Instant damage GameplayEffect every spell/enemy uses (SetByCaller Data.Damage). Assign GE_Damage.
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// --- Pools (authored content) ---
	UPROPERTY(EditAnywhere, Category = "Pools")
	TArray<TSoftObjectPtr<UElementDefinition>> ElementPool;

	UPROPERTY(EditAnywhere, Category = "Pools")
	TArray<TSoftObjectPtr<UFormDefinition>> FormPool;

	UPROPERTY(EditAnywhere, Category = "Pools")
	TArray<TSoftObjectPtr<UModifierDefinition>> ModifierPool;

	// --- Rarity weights (relative; ~60/25/12/3 by default) ---
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float CommonWeight = 60.f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float RareWeight = 25.f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float EpicWeight = 12.f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float LegendaryWeight = 3.f;

	// --- Power multiplier per rarity (drives damage and visual scale) ---
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float CommonPower = 1.f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float RarePower = 1.4f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float EpicPower = 1.9f;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	float LegendaryPower = 2.6f;

	// --- Combo ---
	UPROPERTY(EditAnywhere, Category = "Combo")
	float ComboWindow = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Combo")
	TArray<FElementOpposition> Oppositions;

	// Damage multiplier when both hands cast the same element (amplification).
	UPROPERTY(EditAnywhere, Category = "Combo")
	float SameElementComboMultiplier = 2.f;

	// Damage multiplier when the two elements are opposed (reaction).
	UPROPERTY(EditAnywhere, Category = "Combo")
	float OpposedComboMultiplier = 2.5f;

	// Window for two DIFFERENT players to combo the same enemy with opposed elements (more forgiving
	// than the solo two-hand window since it requires human coordination).
	UPROPERTY(EditAnywhere, Category = "Combo")
	float CrossPlayerComboWindow = 1.f;

	// Radius of the cross-player reaction burst around the struck enemy, in cm.
	UPROPERTY(EditAnywhere, Category = "Combo")
	float CrossPlayerComboRadius = 350.f;

	float GetPowerForRarity(ESpellRarity Rarity) const;

	// True if A and B form an opposed pair (order-independent).
	bool AreElementsOpposed(const FGameplayTag& A, const FGameplayTag& B) const;
};
