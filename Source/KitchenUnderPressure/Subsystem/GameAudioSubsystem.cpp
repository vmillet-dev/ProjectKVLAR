// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAudioSubsystem.h"
#include "AudioSaveGame.h"
#include "Settings/GameAudioSettings.h"
#include "Settings/GameMapSettings.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectGlobals.h"

void UGameAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSettings();

	// Pick the right music every time a map finishes loading. PostLoadMapWithWorld covers level
	// travels (menu <-> game), while OnWorldInitializedActors covers the very first map — which
	// PostLoadMapWithWorld is not broadcast for at PIE start.
	MapLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGameAudioSubsystem::HandleMapLoaded);
	WorldActorsInitHandle = FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UGameAudioSubsystem::HandleWorldActorsInitialized);
}

void UGameAudioSubsystem::Deinitialize()
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

const UGameAudioSettings* UGameAudioSubsystem::GetSettings() const
{
	return GetDefault<UGameAudioSettings>();
}

UWorld* UGameAudioSubsystem::GetAudioWorld() const
{
	return GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
}

void UGameAudioSubsystem::LoadSettings()
{
	if (UAudioSaveGame* Save = Cast<UAudioSaveGame>(UGameplayStatics::LoadGameFromSlot(UAudioSaveGame::SlotName, 0)))
	{
		MasterVolume = FMath::Clamp(Save->MasterVolume, 0.f, 1.f);
		MusicVolume = FMath::Clamp(Save->MusicVolume, 0.f, 1.f);
		SfxVolume = FMath::Clamp(Save->SfxVolume, 0.f, 1.f);
	}
	// No save yet: keep the 1.0 defaults.
}

void UGameAudioSubsystem::SaveSettings()
{
	UAudioSaveGame* Save = Cast<UAudioSaveGame>(UGameplayStatics::CreateSaveGameObject(UAudioSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}

	Save->MasterVolume = MasterVolume;
	Save->MusicVolume = MusicVolume;
	Save->SfxVolume = SfxVolume;
	UGameplayStatics::SaveGameToSlot(Save, UAudioSaveGame::SlotName, 0);
}

void UGameAudioSubsystem::SetMasterVolume(float Value)
{
	MasterVolume = FMath::Clamp(Value, 0.f, 1.f);
	ApplyMusicVolume();
}

void UGameAudioSubsystem::SetMusicVolume(float Value)
{
	MusicVolume = FMath::Clamp(Value, 0.f, 1.f);
	ApplyMusicVolume();
}

void UGameAudioSubsystem::SetSfxVolume(float Value)
{
	// UI sounds are fire-and-forget; the new level is read the next time one plays.
	SfxVolume = FMath::Clamp(Value, 0.f, 1.f);
}

void UGameAudioSubsystem::ApplyMusicVolume()
{
	if (MusicComp)
	{
		MusicComp->SetVolumeMultiplier(MusicLevel());
	}
}

void UGameAudioSubsystem::HandleMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		return;
	}

	const UGameAudioSettings* S = GetSettings();
	if (!S)
	{
		return;
	}

	// Gameplay map gets the gameplay track; menu, lobby and anything else share the menu track.
	// Compare on short names so the PIE package prefix ("UEDPIE_0_") does not break the match.
	const UGameMapSettings* Maps = GetDefault<UGameMapSettings>();
	const FString LoadedShort = FPackageName::GetShortName(UWorld::RemovePIEPrefix(LoadedWorld->GetOutermost()->GetName()));
	const FString GameplayShort = Maps ? FPackageName::GetShortName(Maps->GetGameplayMapName()) : FString();
	const bool bGameplay = !GameplayShort.IsEmpty() && LoadedShort == GameplayShort;

	USoundBase* Desired = bGameplay ? S->GameplayMusic.LoadSynchronous() : S->MenuMusic.LoadSynchronous();
	PlayMusic(LoadedWorld, Desired);
}

void UGameAudioSubsystem::HandleWorldActorsInitialized(const FActorsInitializedParams& Params)
{
	// Only react to our own game/PIE world; ignore editor preview worlds and other instances.
	// PlayMusic is idempotent, so overlapping with PostLoadMapWithWorld on travels is harmless.
	UWorld* World = Params.World;
	if (World && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance())
	{
		HandleMapLoaded(World);
	}
}

void UGameAudioSubsystem::PlayMusic(UWorld* World, USoundBase* Music)
{
	// Same track already playing (it survived the level change) → leave it running untouched.
	if (Music == CurrentMusic && MusicComp && MusicComp->IsPlaying())
	{
		return;
	}

	const UGameAudioSettings* S = GetSettings();
	const float Fade = S ? S->MusicFadeSeconds : 1.f;

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

	// Persist across level transitions so menu -> lobby does not restart the music; we manage
	// the lifetime ourselves (bAutoDestroy = false). Volume is Master * Music.
	MusicComp = UGameplayStatics::SpawnSound2D(World, Music, MusicLevel(), 1.f, 0.f, nullptr, true, false);
	if (MusicComp)
	{
		MusicComp->FadeIn(Fade, MusicLevel());
	}
}

void UGameAudioSubsystem::PlayMenuSound(EMenuSound Sound)
{
	const UGameAudioSettings* S = GetSettings();
	if (!S)
	{
		return;
	}

	USoundBase* Cue = nullptr;
	switch (Sound)
	{
	case EMenuSound::Navigate: Cue = S->NavigateSound.LoadSynchronous(); break;
	case EMenuSound::Accept:   Cue = S->AcceptSound.LoadSynchronous();   break;
	case EMenuSound::Back:     Cue = S->BackSound.LoadSynchronous();     break;
	}

	if (Cue)
	{
		if (UWorld* World = GetAudioWorld())
		{
			UGameplayStatics::PlaySound2D(World, Cue, SfxLevel());
		}
	}
}
