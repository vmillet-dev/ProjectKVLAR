// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "LobbyPlayerController.h"
#include "LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Settings/GameMapSettings.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
	DefaultPawnClass = nullptr; // no pawn in the waiting room
	
	bUseSeamlessTravel = true;
}

bool ALobbyGameMode::AreAllPlayersReady() const
{
	if (!GameState || GameState->PlayerArray.Num() == 0)
	{
		return false;
	}

	for (const APlayerState* PS : GameState->PlayerArray)
	{
		const ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS);
		if (!LobbyPS || !LobbyPS->bIsReady)
		{
			return false;
		}
	}
	return true;
}

void ALobbyGameMode::TryStartMatch()
{
	if (!HasAuthority() || !AreAllPlayersReady())
	{
		return;
	}

	// ServerTravel carries every connected client to the gameplay map together,
	// keeping the host as a listen server.
	const UGameMapSettings* MapSettings = GetDefault<UGameMapSettings>();
	const FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapSettings->GetGameplayMapName());
	GetWorld()->ServerTravel(TravelURL);
}
