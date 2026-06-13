// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameMapSettings.generated.h"

/**
 * Project-wide map references for navigation between menu, lobby and gameplay.
 * Editable under Project Settings > Game > "Game Maps"; persisted to DefaultGame.ini.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game Maps"))
class KITCHENUNDERPRESSURE_API UGameMapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameMapSettings();

	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	UPROPERTY(EditAnywhere, config, Category = "Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> MainMenuMap;

	UPROPERTY(EditAnywhere, config, Category = "Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> LobbyMap;

	UPROPERTY(EditAnywhere, config, Category = "Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> GameplayMap;

	// Long package name ("/Game/Maps/Lvl_X") expected by OpenLevel / ServerTravel.
	FString GetMainMenuMapName() const { return MapToPackageName(MainMenuMap); }
	FString GetLobbyMapName() const { return MapToPackageName(LobbyMap); }
	FString GetGameplayMapName() const { return MapToPackageName(GameplayMap); }

private:
	static FString MapToPackageName(const TSoftObjectPtr<UWorld>& Map)
	{
		return Map.ToSoftObjectPath().GetLongPackageName();
	}
};
