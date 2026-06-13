// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * Server-authoritative game mode for the waiting room. Tracks connected players and,
 * on host request, travels everyone to the gameplay map once all players are ready.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	// True when at least one player is connected and all of them are ready.
	bool AreAllPlayersReady() const;

	// If everyone is ready, ServerTravel all connected players to the gameplay map.
	void TryStartMatch();
};
