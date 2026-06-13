// Copyright Epic Games, Inc. All Rights Reserved.


#include "KitchenUnderPressurePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "KitchenUnderPressureCameraManager.h"
#include "MenuInputComponent.h"
#include "Input/InputConfig.h"
#include "Blueprint/UserWidget.h"
#include "KitchenUnderPressure.h"
#include "UI/InGameMenuWidget.h"
#include "Kismet/GameplayStatics.h"

AKitchenUnderPressurePlayerController::AKitchenUnderPressurePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AKitchenUnderPressureCameraManager::StaticClass();

	MenuInput = CreateDefaultSubobject<UMenuInputComponent>(TEXT("MenuInput"));
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

		// Bind the menu IMC actions once; they only act while a menu is activated.
		if (MenuInput)
		{
			MenuInput->Setup(this);
		}
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

	// Create the widget once and reuse it across opens/closes.
	if (!InGameMenu)
	{
		InGameMenu = CreateWidget<UInGameMenuWidget>(this, InGameMenuClass);
	}
	if (!InGameMenu)
	{
		return;
	}

	InGameMenu->AddToViewport();
	bMenuOpen = true;

	// Solo (standalone) freezes the world; multiplayer keeps running. GameAndUI lets
	// the menu's toggle action still fire so Escape can also close the menu.
	if (GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Priority 1 puts the menu IMC above gameplay (0) so it consumes the shared
	// stick/accept keys — the character does not move or jump while the menu is open.
	if (MenuInput)
	{
		MenuInput->ActivateMenu(InGameMenu, 1);
	}
}

void AKitchenUnderPressurePlayerController::CloseInGameMenu()
{
	if (InGameMenu)
	{
		InGameMenu->RemoveFromParent();
	}
	bMenuOpen = false;

	// Stop forwarding input and remove the menu IMC so gameplay input is restored.
	if (MenuInput)
	{
		MenuInput->DeactivateMenu();
	}

	if (GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(this, false);
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
