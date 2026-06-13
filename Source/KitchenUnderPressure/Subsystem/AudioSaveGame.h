// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AudioSaveGame.generated.h"

/**
 * Persisted player audio preferences (the three volume sliders, 0..1). Stored in a single
 * save slot and loaded once by UGameAudioSubsystem on game start.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAudioSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Save slot name used by UGameAudioSubsystem (defined in the .cpp).
	static const FString SlotName;

	UPROPERTY()
	float MasterVolume = 1.f;

	UPROPERTY()
	float MusicVolume = 1.f;

	UPROPERTY()
	float SfxVolume = 1.f;
};
