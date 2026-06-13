// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KUPUISettings.generated.h"

class UKUPPrimaryLayout;

/**
 * Project-wide UI references (Project Settings > Game > "Game UI"; persisted to DefaultGame.ini).
 * Holds the primary layout widget class that UKUPUISubsystem instantiates once per local player.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game UI"))
class KITCHENUNDERPRESSURE_API UKUPUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	// WBP deriving from UKUPPrimaryLayout, added to the viewport as the menu shell.
	UPROPERTY(EditAnywhere, config, Category = "UI", meta = (MetaClass = "/Script/KitchenUnderPressure.KUPPrimaryLayout"))
	TSoftClassPtr<UKUPPrimaryLayout> PrimaryLayout;
};
