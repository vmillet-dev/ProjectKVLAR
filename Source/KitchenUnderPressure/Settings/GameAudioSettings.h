// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameAudioSettings.generated.h"

class USoundBase;

/**
 * Project-wide references to the audio assets driven by code: the menu/gameplay music tracks
 * and the UI feedback sounds (navigate / accept / back). Editable under Project Settings >
 * Game > "Game Audio"; persisted to DefaultGame.ini. Every reference is optional: an unset
 * slot simply means that piece of audio is silent (UGameAudioSubsystem is null-safe). Volume
 * is applied directly by the subsystem, so no sound class / sound mix setup is required.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game Audio"))
class KITCHENUNDERPRESSURE_API UGameAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	// Played on the main menu and the lobby (should be a looping asset).
	UPROPERTY(EditAnywhere, config, Category = "Music")
	TSoftObjectPtr<USoundBase> MenuMusic;

	// Played on the gameplay map (should be a looping asset).
	UPROPERTY(EditAnywhere, config, Category = "Music")
	TSoftObjectPtr<USoundBase> GameplayMusic;

	// Cross-fade duration when switching music tracks.
	UPROPERTY(EditAnywhere, config, Category = "Music", meta = (ClampMin = "0.0"))
	float MusicFadeSeconds = 1.f;

	UPROPERTY(EditAnywhere, config, Category = "UI Sounds")
	TSoftObjectPtr<USoundBase> NavigateSound;

	UPROPERTY(EditAnywhere, config, Category = "UI Sounds")
	TSoftObjectPtr<USoundBase> AcceptSound;

	UPROPERTY(EditAnywhere, config, Category = "UI Sounds")
	TSoftObjectPtr<USoundBase> BackSound;
};
