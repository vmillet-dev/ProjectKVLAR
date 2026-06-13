// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameAudioSubsystem.generated.h"

class UGameAudioSettings;
class USoundBase;
class UAudioComponent;
struct FActorsInitializedParams;

// Shared UI feedback sounds, played from UMenuNavWidget on navigation / validation / back.
enum class EMenuSound : uint8
{
	Navigate,
	Accept,
	Back
};

/**
 * Central audio manager living on the game instance (persists across maps). Owns the three
 * volume values (Master / Music / SFX) and applies them directly: Master*Music scales the
 * background music component it spawns, Master*SFX scales each UI sound it plays. It swaps the
 * music when the map changes (menu & lobby share one track, gameplay uses another). All asset
 * references come from UGameAudioSettings and are null-safe.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UGameAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Volume getters (0..1), used to initialise the option sliders.
	float GetMasterVolume() const { return MasterVolume; }
	float GetMusicVolume() const { return MusicVolume; }
	float GetSfxVolume() const { return SfxVolume; }

	// Apply a volume live. Does NOT write to disk so a mouse drag does not hammer the save
	// file; call SaveSettings() once the value is committed.
	void SetMasterVolume(float Value);
	void SetMusicVolume(float Value);
	void SetSfxVolume(float Value);

	// Persist the current volumes (call on slider release / after a gamepad step).
	void SaveSettings();

	// Play one of the shared UI feedback sounds at the current SFX volume (null-safe).
	void PlayMenuSound(EMenuSound Sound);

private:
	void LoadSettings();
	void ApplyMusicVolume();
	void HandleMapLoaded(UWorld* LoadedWorld);
	// Covers the initial map (PostLoadMapWithWorld is not broadcast for the first PIE world).
	void HandleWorldActorsInitialized(const FActorsInitializedParams& Params);
	void PlayMusic(UWorld* World, USoundBase* Music);

	const UGameAudioSettings* GetSettings() const;
	UWorld* GetAudioWorld() const;

	// Effective playback levels (the two sliders combined with Master).
	float MusicLevel() const { return MasterVolume * MusicVolume; }
	float SfxLevel() const { return MasterVolume * SfxVolume; }

	float MasterVolume = 1.f;
	float MusicVolume = 1.f;
	float SfxVolume = 1.f;

	// Currently playing music track and its component (kept across level transitions).
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicComp;

	UPROPERTY()
	TObjectPtr<USoundBase> CurrentMusic;

	FDelegateHandle MapLoadedHandle;
	FDelegateHandle WorldActorsInitHandle;
};
