// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "UI/LobbyWidget.h"
#include "UI/KUPUISubsystem.h"
#include "OnlineSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/GameMapSettings.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !LobbyWidgetClass)
	{
		return;
	}

	// CommonUI: push the lobby screen onto the primary layout; CommonUI owns focus and input routing.
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UKUPUISubsystem* UI = LP->GetSubsystem<UKUPUISubsystem>())
		{
			UI->PushScreen(LobbyWidgetClass);
		}
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bReady)
{
	// Runs on the server; the replicated flag then propagates to every client.
	if (ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>())
	{
		LobbyPS->bIsReady = bReady;
	}
}

void ALobbyPlayerController::ServerRequestStart_Implementation()
{
	// GetAuthGameMode is only valid on the server, which is exactly where we are.
	if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		GM->TryStartMatch();
	}
}

void ALobbyPlayerController::LeaveLobby()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* Online = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			// Destroy the session first; the subsystem travels back to the menu only once it
			// is actually gone, so EOS does not keep us registered ("already in session").
			Online->LeaveGame(/*bReturnToMenu=*/true);
			return;
		}
	}

	// Fallback if the online subsystem is somehow unavailable.
	const UGameMapSettings* MapSettings = GetDefault<UGameMapSettings>();
	UGameplayStatics::OpenLevel(this, FName(*MapSettings->GetMainMenuMapName()));
}
