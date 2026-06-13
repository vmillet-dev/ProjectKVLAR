// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "KUPGameUserSettings.generated.h"

/**
 * Project GameUserSettings: the standard home for persisted player options (currently the three
 * audio volumes, 0..1). Registered via [/Script/Engine.Engine] GameUserSettingsClassName so the
 * engine creates this subclass; values persist to GameUserSettings.ini. UAudioSubsystem reads
 * and writes them and pushes the volumes into the audio control buses.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UKUPGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	// The engine-owned instance (null only before engine init).
	static UKUPGameUserSettings* Get();

	float GetMasterVolume() const { return MasterVolume; }
	float GetMusicVolume() const { return MusicVolume; }
	float GetSfxVolume() const { return SfxVolume; }

	void SetMasterVolume(float Value) { MasterVolume = FMath::Clamp(Value, 0.f, 1.f); }
	void SetMusicVolume(float Value) { MusicVolume = FMath::Clamp(Value, 0.f, 1.f); }
	void SetSfxVolume(float Value) { SfxVolume = FMath::Clamp(Value, 0.f, 1.f); }

private:
	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(config)
	float MusicVolume = 1.f;

	UPROPERTY(config)
	float SfxVolume = 1.f;
};
