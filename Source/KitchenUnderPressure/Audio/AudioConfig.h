// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AudioConfig.generated.h"

class USoundBase;
class USoundControlBus;
class USoundControlBusMix;

/**
 * Authored audio content + mixing references: the menu/gameplay music and the Audio Modulation
 * control buses (Master/Music/SFX) plus the mix that carries their values. UAudioSubsystem reads the
 * active config (selected by UKUPAudioSettings) and pushes the player's slider values into the buses;
 * the per-bus volume scaling happens through each sound's SoundClass routing (set up in the editor),
 * so no code multiplies volumes by hand. UI button sounds live in the CommonButtonStyle, not here.
 */
UCLASS(BlueprintType)
class KITCHENUNDERPRESSURE_API UAudioConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Music ---
	UPROPERTY(EditAnywhere, Category = "Music")
	TSoftObjectPtr<USoundBase> MenuMusic;

	UPROPERTY(EditAnywhere, Category = "Music")
	TSoftObjectPtr<USoundBase> GameplayMusic;

	// Cross-fade duration when switching music tracks.
	UPROPERTY(EditAnywhere, Category = "Music", meta = (ClampMin = "0.0"))
	float MusicFadeSeconds = 1.f;

	// --- Mixing (Audio Modulation) ---
	// The three buses driven by the volume sliders, and the mix that carries their current values.
	UPROPERTY(EditAnywhere, Category = "Mixing")
	TObjectPtr<USoundControlBus> MasterBus;

	UPROPERTY(EditAnywhere, Category = "Mixing")
	TObjectPtr<USoundControlBus> MusicBus;

	UPROPERTY(EditAnywhere, Category = "Mixing")
	TObjectPtr<USoundControlBus> SfxBus;

	UPROPERTY(EditAnywhere, Category = "Mixing")
	TObjectPtr<USoundControlBusMix> MainMix;
};
