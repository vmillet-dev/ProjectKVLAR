// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlchemistGameMode.h"
#include "AlchemistGameState.h"
#include "AlchemistPlayerState.h"
#include "KUPDebugHUD.h"
#include "KUPGameplayTags.h"
#include "Dungeon/RoomActor.h"
#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

AAlchemistGameMode::AAlchemistGameMode()
{
	GameStateClass = AAlchemistGameState::StaticClass();
	PlayerStateClass = AAlchemistPlayerState::StaticClass();
	// Debug overlay on by default (enemy health bars, local loadout); see KUP.Debug.HUD.
	HUDClass = AKUPDebugHUD::StaticClass();
	// DefaultPawnClass and PlayerControllerClass stay as set in BP_FirstPersonGameMode.
}

void AAlchemistGameMode::BeginPlay()
{
	Super::BeginPlay();
	GatherRooms();
}

void AAlchemistGameMode::InitGameState()
{
	Super::InitGameState();
	if (AAlchemistGameState* GS = GetGameState<AAlchemistGameState>())
	{
		GS->bFriendlyFire = bFriendlyFire;
	}
}

void AAlchemistGameMode::GatherRooms()
{
	Rooms.Reset();
	for (TActorIterator<ARoomActor> It(GetWorld()); It; ++It)
	{
		Rooms.Add(*It);
	}
	Rooms.Sort([](const ARoomActor& A, const ARoomActor& B)
	{
		return A.GetRoomIndex() < B.GetRoomIndex();
	});

	if (AAlchemistGameState* GS = GetGameState<AAlchemistGameState>())
	{
		GS->TotalRooms = Rooms.Num();
		GS->CurrentRoomIndex = 0;
		GS->OnRunProgressChanged.Broadcast();
	}

	for (ARoomActor* Room : Rooms)
	{
		if (Room)
		{
			Room->OnRoomCleared.AddUObject(this, &AAlchemistGameMode::HandleRoomCleared);
		}
	}
}

void AAlchemistGameMode::HandleRoomCleared(ARoomActor* Room)
{
	AAlchemistGameState* GS = GetGameState<AAlchemistGameState>();
	if (!GS)
	{
		return;
	}

	GS->CurrentRoomIndex = FMath::Min(GS->CurrentRoomIndex + 1, GS->TotalRooms);
	if (GS->TotalRooms > 0 && GS->CurrentRoomIndex >= GS->TotalRooms)
	{
		GS->RunState = ERunState::Cleared;
	}
	GS->OnRunProgressChanged.Broadcast();

	if (GEngine)
	{
		const FString Message = (GS->RunState == ERunState::Cleared)
			? TEXT("RUN CLEARED!")
			: FString::Printf(TEXT("Room cleared  -  %d / %d"), GS->CurrentRoomIndex, GS->TotalRooms);
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, Message);
	}
}

void AAlchemistGameMode::NotifyPlayerDied(AController* DeadPlayer)
{
	AAlchemistGameState* GS = GetGameState<AAlchemistGameState>();
	if (!GS || GS->RunState != ERunState::InProgress)
	{
		return;
	}

	// Recount the living on every death instead of keeping a counter, so players joining, leaving or
	// disconnecting mid-run can never desynchronise the wipe detection. The dying player's State.Dead
	// tag is set before this is called.
	for (const APlayerState* PlayerState : GS->PlayerArray)
	{
		const AAlchemistPlayerState* AlchemistPS = Cast<AAlchemistPlayerState>(PlayerState);
		const UAbilitySystemComponent* ASC = AlchemistPS ? AlchemistPS->GetAbilitySystemComponent() : nullptr;
		if (ASC && !ASC->HasMatchingGameplayTag(KUPTags::State_Dead))
		{
			return; // someone is still alive
		}
	}

	GS->RunState = ERunState::Wiped;
	GS->OnRunProgressChanged.Broadcast();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, TEXT("PARTY WIPED"));
	}
}
