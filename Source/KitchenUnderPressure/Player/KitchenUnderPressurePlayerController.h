// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KitchenUnderPressurePlayerController.generated.h"

class UInGameMenuWidget;
class UInputConfig;

/**
 *  Simple first person Player Controller.
 *  Owns the gameplay input mapping context (via UInputConfig) and the in-game pause menu,
 *  and overrides the Player Camera Manager class.
 */
UCLASS(abstract)
class KITCHENUNDERPRESSURE_API AKitchenUnderPressurePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Constructor */
	AKitchenUnderPressurePlayerController();

	/** Opens/closes the in-game menu, pausing the world only when standalone (solo).
	 *  Called by the character's pause Input Action. */
	void ToggleInGameMenu();

	/** Closes the in-game menu and restores game input. Called by the Continue button. */
	void CloseInGameMenu();

	/** Gameplay input config (mapping contexts + actions). The pawn reads its actions from it. */
	const UInputConfig* GetInputConfig() const { return InputConfig; }

protected:

	/** Gameplay input: mapping context(s) added on this controller plus the actions the pawn binds. */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputConfig> InputConfig;

	/** In-game pause/options menu widget class. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInGameMenuWidget> InGameMenuClass;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Adds the gameplay mapping context(s) from InputConfig */
	virtual void SetupInputComponent() override;

private:
	// The pushed pause screen while the menu is open (popped on close).
	UPROPERTY()
	TObjectPtr<UInGameMenuWidget> InGameMenu;

	bool bMenuOpen = false;
};
