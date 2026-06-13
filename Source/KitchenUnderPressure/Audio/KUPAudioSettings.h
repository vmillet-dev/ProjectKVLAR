// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KUPAudioSettings.generated.h"

class UAudioConfig;

/**
 * Project-wide pointer to the active audio configuration (Project Settings > Game > "Game Audio";
 * persisted to DefaultGame.ini). Named with the KUP prefix because the engine already defines a
 * UAudioSettings. The music, UI sounds and control buses live in the UAudioConfig data asset.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game Audio"))
class KITCHENUNDERPRESSURE_API UKUPAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	UPROPERTY(EditAnywhere, config, Category = "Audio")
	TSoftObjectPtr<UAudioConfig> ActiveConfig;
};
