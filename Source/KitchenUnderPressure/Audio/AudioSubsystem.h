// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSubsystem.generated.h"

class UAudioConfig;
class USoundBase;
class USoundControlBus;
class UAudioComponent;
struct FActorsInitializedParams;

/**
 * Central audio manager living on the game instance (persists across maps). Reads the active
 * UAudioConfig (selected by UKUPAudioSettings) for its music and Audio Modulation control buses.
 * Volume is driven entirely by the buses: the three slider values are pushed into the Master / Music
 * / SFX buses through the config's control bus mix, and every sound routed to those buses (via its
 * SoundClass) scales automatically — no manual volume multiplication. Player volumes persist in
 * UKUPGameUserSettings. The music track swaps when the map changes.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Volume getters (0..1), used to initialise the option sliders.
	float GetMasterVolume() const;
	float GetMusicVolume() const;
	float GetSfxVolume() const;

	// Apply a volume live to its control bus. Does NOT write to disk so a mouse drag does not hammer
	// the file; call SaveSettings() once the value is committed.
	void SetMasterVolume(float Value);
	void SetMusicVolume(float Value);
	void SetSfxVolume(float Value);

	// Persist the current volumes (call on slider release / after a gamepad step).
	void SaveSettings();

private:
	void HandleMapLoaded(UWorld* LoadedWorld);
	// Covers the initial map (PostLoadMapWithWorld is not broadcast for the first PIE world).
	void HandleWorldActorsInitialized(const FActorsInitializedParams& Params);
	void PlayMusic(UWorld* World, USoundBase* Music);

	// Activate the bus mix once an audio world exists, then push the saved volumes into the buses.
	void EnsureMixActivated();
	void ApplyBusVolume(USoundControlBus* Bus, float Value);

	// Loads and caches the active config on first use (the hard ref also keeps its buses/mix alive).
	const UAudioConfig* GetAudioConfig();
	UWorld* GetAudioWorld() const;

	UPROPERTY()
	TObjectPtr<UAudioConfig> ConfigCache;

	// Currently playing music track and its component (kept across level transitions).
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicComp;

	UPROPERTY()
	TObjectPtr<USoundBase> CurrentMusic;

	bool bMixActivated = false;

	FDelegateHandle MapLoadedHandle;
	FDelegateHandle WorldActorsInitHandle;
};
