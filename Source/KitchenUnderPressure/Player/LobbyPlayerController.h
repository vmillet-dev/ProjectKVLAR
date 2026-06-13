// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyWidget;

/**
 * Player controller for the waiting room. Pushes the lobby UI onto the local player's CommonUI
 * primary layout, and relays ready/start requests to the server, plus leaving back to the main menu.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets this player's ready flag on the server (server has authority over PlayerState).
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bReady);

	// Asks the server (host) to start the match if everyone is ready.
	UFUNCTION(Server, Reliable)
	void ServerRequestStart();

	// Local: destroy/leave the session and return to the main-menu map.
	void LeaveLobby();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;
};
