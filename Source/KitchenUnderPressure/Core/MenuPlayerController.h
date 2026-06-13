// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

class UMainMenuWidget;
class UMenuInputComponent;

/**
 * Player controller for the main-menu map. Creates the main-menu widget on the local
 * client and drives it with mouse, keyboard or gamepad through the menu input component.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMenuPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuClass;

private:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenu;

	UPROPERTY()
	TObjectPtr<UMenuInputComponent> MenuInput;
};
