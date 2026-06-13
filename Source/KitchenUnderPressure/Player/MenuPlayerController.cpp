// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "UI/MainMenuWidget.h"
#include "UI/KUPUISubsystem.h"
#include "OnlineSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Defensive: a crash, hard quit or closed host can leave a registered EOS session behind,
	// which would make the next Join fail with "already in session". Drop any leftover now.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* Online = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			Online->CleanupLingeringSession();
		}
	}

	if (!IsLocalController() || !MainMenuClass)
	{
		return;
	}

	// CommonUI: push the main menu onto the primary layout. The UI subsystem owns the widget, and
	// CommonUI handles focus, gamepad/keyboard/mouse routing, the Back action, the input mode and the
	// cursor (shown for mouse, hidden on gamepad) via the screen's GetDesiredInputConfig.
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UKUPUISubsystem* UI = LP->GetSubsystem<UKUPUISubsystem>())
		{
			UI->PushScreen(MainMenuClass);
		}
	}
}
