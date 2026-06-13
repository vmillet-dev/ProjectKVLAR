// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

class UMainMenuWidget;

/**
 * Player controller for the main-menu map. Pushes the main-menu screen onto the local player's
 * CommonUI primary layout (via UKUPUISubsystem); focus, gamepad/keyboard/mouse routing and Back
 * are handled by CommonUI.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuClass;
};
