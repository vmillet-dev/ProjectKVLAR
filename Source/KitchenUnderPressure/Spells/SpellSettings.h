// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SpellSettings.generated.h"

class USpellConfig;

/**
 * Project-wide pointer to the active spell configuration (Project Settings > Game > "Alchemist
 * Spells"; persisted to DefaultGame.ini). The pools, rarity and combo tuning live in the
 * USpellConfig data asset itself; USpellRegistrySubsystem loads it on first use.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Alchemist Spells"))
class KITCHENUNDERPRESSURE_API USpellSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	UPROPERTY(EditAnywhere, config, Category = "Spells")
	TSoftObjectPtr<USpellConfig> ActiveConfig;
};
