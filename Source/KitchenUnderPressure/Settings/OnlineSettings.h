// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OnlineSettings.generated.h"

/**
 * Project-wide EOS/online configuration. Keeps login and session constants out of the C++ so
 * they are editable under Project Settings > Game > "Online" and consistent with the other
 * settings classes (audio, maps, menu input). Persisted to DefaultGame.ini.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Online"))
class KITCHENUNDERPRESSURE_API UOnlineSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	// OSS credentials type passed to Login (e.g. "Developer", "AccountPortal", "PersistentAuth").
	UPROPERTY(config, EditAnywhere, Category = "Login")
	FString LoginType = TEXT("Developer");

	// Address of the EOS Developer Auth Tool (host:port), used by the "Developer" login type.
	UPROPERTY(config, EditAnywhere, Category = "Login")
	FString DevAuthAddress = TEXT("127.0.0.1:6300");

	// Value tagged on hosted sessions and matched when searching, so we only find this game.
	UPROPERTY(config, EditAnywhere, Category = "Session")
	FString GameKey = TEXT("MyGameUE57");
};
