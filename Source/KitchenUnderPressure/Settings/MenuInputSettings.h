// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MenuInputSettings.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Project-wide references to the menu navigation input assets (IMC + 3 actions).
 * Set once in Project Settings > Game > "Menu Input" so the menu controllers do not
 * each need a Blueprint subclass just to carry these asset pointers.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Menu Input"))
class KITCHENUNDERPRESSURE_API UMenuInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName("Game"); }

	// Mapping context added while a menu is active (gamepad + keyboard navigation).
	UPROPERTY(config, EditAnywhere, Category = "Menu Input")
	TSoftObjectPtr<UInputMappingContext> MenuContext;

	// Axis1D: > 0 moves up (previous), < 0 moves down (next).
	UPROPERTY(config, EditAnywhere, Category = "Menu Input")
	TSoftObjectPtr<UInputAction> NavigateAction;

	// Axis1D: > 0 increases the focused slider, < 0 decreases it (gamepad left stick X / arrows).
	UPROPERTY(config, EditAnywhere, Category = "Menu Input")
	TSoftObjectPtr<UInputAction> AdjustAction;

	// Digital: validate the highlighted button (gamepad A / Enter).
	UPROPERTY(config, EditAnywhere, Category = "Menu Input")
	TSoftObjectPtr<UInputAction> AcceptAction;

	// Digital: go back one level / close (gamepad B / Escape).
	UPROPERTY(config, EditAnywhere, Category = "Menu Input")
	TSoftObjectPtr<UInputAction> BackAction;
};
