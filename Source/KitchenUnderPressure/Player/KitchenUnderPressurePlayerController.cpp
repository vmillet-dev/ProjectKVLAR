// Copyright Epic Games, Inc. All Rights Reserved.


#include "KitchenUnderPressurePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "KitchenUnderPressureCameraManager.h"
#include "Input/InputConfig.h"
#include "KitchenUnderPressure.h"
#include "UI/InGameMenuWidget.h"
#include "UI/KUPUISubsystem.h"
#include "Kismet/GameplayStatics.h"

AKitchenUnderPressurePlayerController::AKitchenUnderPressurePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AKitchenUnderPressureCameraManager::StaticClass();
}

void AKitchenUnderPressurePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Input mode is viewport-global state, not per-controller: a menu/lobby controller may have
	// left the viewport in UI-only mode before traveling here. Restore gameplay input on the
	// local controller so controls respond after arriving from a menu.
	if (IsLocalPlayerController())
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void AKitchenUnderPressurePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Only add the gameplay context(s) for the local player controller.
	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	if (!InputConfig)
	{
		UE_LOG(LogKitchenUnderPressure, Warning, TEXT("'%s' has no InputConfig assigned; gameplay input is disabled."), *GetNameSafe(this));
		return;
	}

	for (UInputMappingContext* Context : InputConfig->MappingContexts)
	{
		if (Context)
		{
			Subsystem->AddMappingContext(Context, InputConfig->Priority);
		}
	}
}

void AKitchenUnderPressurePlayerController::ToggleInGameMenu()
{
	if (bMenuOpen)
	{
		CloseInGameMenu();
		return;
	}

	if (!InGameMenuClass)
	{
		return;
	}

	// Push the pause screen onto the local player's CommonUI primary layout.
	ULocalPlayer* LP = GetLocalPlayer();
	UKUPUISubsystem* UI = LP ? LP->GetSubsystem<UKUPUISubsystem>() : nullptr;
	if (!UI)
	{
		return;
	}

	InGameMenu = Cast<UInGameMenuWidget>(UI->PushScreen(InGameMenuClass));
	if (!InGameMenu)
	{
		return;
	}
	bMenuOpen = true;

	// CommonUI applies the menu input config (input routed to UI, cursor hidden on gamepad) when the
	// screen activates. Solo (standalone) freezes the world; multiplayer keeps running.
	if (GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AKitchenUnderPressurePlayerController::CloseInGameMenu()
{
	// Pop the pause screen off the activatable stack (CommonUI does not auto-restore the input
	// config when the stack empties, so we set game-only input explicitly below).
	if (InGameMenu)
	{
		InGameMenu->DeactivateWidget();
		InGameMenu = nullptr;
	}
	bMenuOpen = false;

	if (GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(this, false);
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
