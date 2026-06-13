// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "MenuInputComponent.h"
#include "UI/MainMenuWidget.h"
#include "OnlineSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"

AMenuPlayerController::AMenuPlayerController()
{
	MenuInput = CreateDefaultSubobject<UMenuInputComponent>(TEXT("MenuInput"));
}

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

	MainMenu = CreateWidget<UMainMenuWidget>(this, MainMenuClass);
	if (MainMenu)
	{
		MainMenu->AddToViewport();

		// GameAndUI (not UI-only) so Enhanced Input still delivers the menu IMC actions.
		// Keep the mouse usable; the device subsystem hides the cursor on gamepad.
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		if (MenuInput)
		{
			MenuInput->Setup(this);
			MenuInput->ActivateMenu(MainMenu);
		}
	}
}
