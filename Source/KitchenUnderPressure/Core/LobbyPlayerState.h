// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

/**
 * Player state for the waiting room. Holds the replicated "ready" flag so every client
 * can display who is ready before the host starts the match.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Replicated to all clients so the lobby UI can show each player's ready state.
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
	bool bIsReady = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_IsReady();
};
