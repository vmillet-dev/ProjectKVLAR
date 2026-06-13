// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioSubsystem.h"
#include "AudioConfig.h"
#include "KUPAudioSettings.h"
#include "Settings/KUPGameUserSettings.h"
#include "Settings/GameMapSettings.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "AudioModulationStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

void UAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Pick the right music every time a map finishes loading. PostLoadMapWithWorld covers level
	// travels (menu <-> game), while OnWorldInitializedActors covers the very first map — which
	// PostLoadMapWithWorld is not broadcast for at PIE start.
	MapLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAudioSubsystem::HandleMapLoaded);
	WorldActorsInitHandle = FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UAudioSubsystem::HandleWorldActorsInitialized);
}

void UAudioSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadedHandle);
	MapLoadedHandle.Reset();
	FWorldDelegates::OnWorldInitializedActors.Remove(WorldActorsInitHandle);
	WorldActorsInitHandle.Reset();

	if (MusicComp)
	{
		MusicComp->Stop();
		MusicComp = nullptr;
	}

	Super::Deinitialize();
}

const UAudioConfig* UAudioSubsystem::GetAudioConfig()
{
	if (!ConfigCache)
	{
		const UKUPAudioSettings* Settings = GetDefault<UKUPAudioSettings>();
		ConfigCache = Settings ? Settings->ActiveConfig.LoadSynchronous() : nullptr;
	}
	return ConfigCache;
}

UWorld* UAudioSubsystem::GetAudioWorld() const
{
	return GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
}

float UAudioSubsystem::GetMasterVolume() const
{
	const UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get();
	return GUS ? GUS->GetMasterVolume() : 1.f;
}

float UAudioSubsystem::GetMusicVolume() const
{
	const UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get();
	return GUS ? GUS->GetMusicVolume() : 1.f;
}

float UAudioSubsystem::GetSfxVolume() const
{
	const UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get();
	return GUS ? GUS->GetSfxVolume() : 1.f;
}

void UAudioSubsystem::SetMasterVolume(float Value)
{
	if (UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get())
	{
		GUS->SetMasterVolume(Value);
		if (const UAudioConfig* Config = GetAudioConfig())
		{
			ApplyBusVolume(Config->MasterBus, GUS->GetMasterVolume());
		}
	}
}

void UAudioSubsystem::SetMusicVolume(float Value)
{
	if (UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get())
	{
		GUS->SetMusicVolume(Value);
		if (const UAudioConfig* Config = GetAudioConfig())
		{
			ApplyBusVolume(Config->MusicBus, GUS->GetMusicVolume());
		}
	}
}

void UAudioSubsystem::SetSfxVolume(float Value)
{
	if (UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get())
	{
		GUS->SetSfxVolume(Value);
		if (const UAudioConfig* Config = GetAudioConfig())
		{
			ApplyBusVolume(Config->SfxBus, GUS->GetSfxVolume());
		}
	}
}

void UAudioSubsystem::SaveSettings()
{
	if (UKUPGameUserSettings* GUS = UKUPGameUserSettings::Get())
	{
		GUS->SaveSettings();
	}
}

void UAudioSubsystem::ApplyBusVolume(USoundControlBus* Bus, float Value)
{
	UWorld* World = GetAudioWorld();
	const UAudioConfig* Config = GetAudioConfig();
	if (!World || !Bus || !Config || !Config->MainMix)
	{
		return;
	}

	// Set this bus's stage in the mix to the slider value; everything routed through the bus follows.
	TArray<FSoundControlBusMixStage> Stages;
	Stages.Add(UAudioModulationStatics::CreateBusMixStage(World, Bus, Value));
	UAudioModulationStatics::UpdateMix(World, Config->MainMix, Stages);
}

void UAudioSubsystem::EnsureMixActivated()
{
	if (bMixActivated)
	{
		return;
	}

	UWorld* World = GetAudioWorld();
	const UAudioConfig* Config = GetAudioConfig();
	if (!World || !Config || !Config->MainMix)
	{
		return;
	}

	UAudioModulationStatics::ActivateBusMix(World, Config->MainMix);
	bMixActivated = true;

	// Push the persisted volumes into the buses so the mix starts at the player's saved levels.
	ApplyBusVolume(Config->MasterBus, GetMasterVolume());
	ApplyBusVolume(Config->MusicBus, GetMusicVolume());
	ApplyBusVolume(Config->SfxBus, GetSfxVolume());
}

void UAudioSubsystem::HandleMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		return;
	}

	const UAudioConfig* Config = GetAudioConfig();
	if (!Config)
	{
		return;
	}

	EnsureMixActivated();

	// Gameplay map gets the gameplay track; menu, lobby and anything else share the menu track.
	// Compare on short names so the PIE package prefix ("UEDPIE_0_") does not break the match.
	const UGameMapSettings* Maps = GetDefault<UGameMapSettings>();
	const FString LoadedShort = FPackageName::GetShortName(UWorld::RemovePIEPrefix(LoadedWorld->GetOutermost()->GetName()));
	const FString GameplayShort = Maps ? FPackageName::GetShortName(Maps->GetGameplayMapName()) : FString();
	const bool bGameplay = !GameplayShort.IsEmpty() && LoadedShort == GameplayShort;

	USoundBase* Desired = bGameplay ? Config->GameplayMusic.LoadSynchronous() : Config->MenuMusic.LoadSynchronous();
	PlayMusic(LoadedWorld, Desired);
}

void UAudioSubsystem::HandleWorldActorsInitialized(const FActorsInitializedParams& Params)
{
	// Only react to our own game/PIE world; ignore editor preview worlds and other instances.
	// PlayMusic is idempotent, so overlapping with PostLoadMapWithWorld on travels is harmless.
	UWorld* World = Params.World;
	if (World && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance())
	{
		HandleMapLoaded(World);
	}
}

void UAudioSubsystem::PlayMusic(UWorld* World, USoundBase* Music)
{
	// Same track already playing (it survived the level change) → leave it running untouched.
	if (Music == CurrentMusic && MusicComp && MusicComp->IsPlaying())
	{
		return;
	}

	const UAudioConfig* Config = GetAudioConfig();
	const float Fade = Config ? Config->MusicFadeSeconds : 1.f;

	if (MusicComp)
	{
		// Let the old component clean itself up once the fade-out completes.
		MusicComp->bAutoDestroy = true;
		MusicComp->FadeOut(Fade, 0.f);
		MusicComp = nullptr;
	}

	CurrentMusic = Music;
	if (!Music || !World)
	{
		return;
	}

	// Volume is driven by the Master/Music control buses through the sound's SoundClass routing, so we
	// spawn at unit volume and let the modulation scale it. Persist across level transitions so
	// menu -> lobby does not restart the music; we manage the lifetime ourselves (bAutoDestroy = false).
	MusicComp = UGameplayStatics::SpawnSound2D(World, Music, 1.f, 1.f, 0.f, nullptr, true, false);
	if (MusicComp)
	{
		MusicComp->FadeIn(Fade, 1.f);
	}
}
