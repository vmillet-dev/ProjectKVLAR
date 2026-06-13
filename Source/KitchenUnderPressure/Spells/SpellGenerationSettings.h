// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "Spells/SpellTypes.h"
#include "SpellGenerationSettings.generated.h"

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
 * Project-wide spell generation config (Project Settings > Game > Alchemist Spells). Holds the
 * element/form/modifier pools, rarity weights and power curve, and the combo settings. Also acts as
 * the runtime registry that resolves a spell's tags back to the authored data assets; the pools are
 * soft references loaded once on first use and kept alive by transient hard-ref arrays.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Alchemist Spells"))
class KITCHENUNDERPRESSURE_API USpellGenerationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	/** Instant damage GameplayEffect every spell/enemy uses (SetByCaller Data.Damage). Assign GE_Damage. */
	UPROPERTY(EditAnywhere, config, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// --- Pools (authored content) ---
	UPROPERTY(EditAnywhere, config, Category = "Pools")
	TArray<TSoftObjectPtr<UElementDefinition>> ElementPool;

	UPROPERTY(EditAnywhere, config, Category = "Pools")
	TArray<TSoftObjectPtr<UFormDefinition>> FormPool;

	UPROPERTY(EditAnywhere, config, Category = "Pools")
	TArray<TSoftObjectPtr<UModifierDefinition>> ModifierPool;

	// --- Rarity weights (relative; ~60/25/12/3 by default) ---
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float CommonWeight = 60.f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float RareWeight = 25.f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float EpicWeight = 12.f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float LegendaryWeight = 3.f;

	// --- Power multiplier per rarity (drives damage and visual scale) ---
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float CommonPower = 1.f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float RarePower = 1.4f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float EpicPower = 1.9f;
	UPROPERTY(EditAnywhere, config, Category = "Rarity")
	float LegendaryPower = 2.6f;

	// --- Combo (used from M5) ---
	UPROPERTY(EditAnywhere, config, Category = "Combo")
	float ComboWindow = 0.35f;

	UPROPERTY(EditAnywhere, config, Category = "Combo")
	TArray<FElementOpposition> Oppositions;

	/** Damage multiplier when both hands cast the same element (amplification). */
	UPROPERTY(EditAnywhere, config, Category = "Combo")
	float SameElementComboMultiplier = 2.f;

	/** Damage multiplier when the two elements are opposed (reaction). */
	UPROPERTY(EditAnywhere, config, Category = "Combo")
	float OpposedComboMultiplier = 2.5f;

	/** Window for two DIFFERENT players to combo the same enemy with opposed elements (GDD 8.1).
	 *  More forgiving than the solo two-hand window since it requires human coordination. */
	UPROPERTY(EditAnywhere, config, Category = "Combo")
	float CrossPlayerComboWindow = 1.f;

	/** Radius of the cross-player reaction burst around the struck enemy, in cm. */
	UPROPERTY(EditAnywhere, config, Category = "Combo")
	float CrossPlayerComboRadius = 350.f;

	// --- Registry resolution (lazy; loads the soft pools once) ---
	UElementDefinition* FindElement(const FGameplayTag& Tag);
	UFormDefinition* FindForm(const FGameplayTag& Tag);
	UModifierDefinition* FindModifier(const FGameplayTag& Tag);

	const TArray<TObjectPtr<UElementDefinition>>& GetLoadedElements();
	const TArray<TObjectPtr<UFormDefinition>>& GetLoadedForms();
	const TArray<TObjectPtr<UModifierDefinition>>& GetLoadedModifiers();

	float GetPowerForRarity(ESpellRarity Rarity) const;
	float GetWeightForRarity(ESpellRarity Rarity) const;

	/** True if A and B form an opposed pair (order-independent). */
	bool AreElementsOpposed(const FGameplayTag& A, const FGameplayTag& B) const;

private:
	void EnsureRegistry();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UElementDefinition>> LoadedElements;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFormDefinition>> LoadedForms;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UModifierDefinition>> LoadedModifiers;

	TMap<FGameplayTag, UElementDefinition*> ElementByTag;
	TMap<FGameplayTag, UFormDefinition*> FormByTag;
	TMap<FGameplayTag, UModifierDefinition*> ModifierByTag;

	bool bRegistryBuilt = false;
};
