// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuNavWidget.h"
#include "InGameMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UAudioOptionsWidget;

/**
 * In-game pause/options menu (solo and multiplayer). "Continue" closes the menu (and
 * unpauses in solo); the other buttons leave to the main menu or quit to desktop.
 * Pausing itself is handled by the owning player controller based on net mode.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UInGameMenuWidget : public UMenuNavWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// "Back" closes the options panel if open, otherwise closes the whole menu.
	virtual void HandleBack() override;

	UPROPERTY(meta = (BindWidget)) UButton* ContinueButton;
	UPROPERTY(meta = (BindWidget)) UButton* OptionsButton;
	UPROPERTY(meta = (BindWidget)) UButton* MainMenuButton;
	UPROPERTY(meta = (BindWidget)) UButton* QuitButton;

	// Optional: switch between the main button list (0) and the options panel (1).
	UPROPERTY(meta = (BindWidgetOptional)) UWidgetSwitcher* MenuSwitcher;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* OptionsBackButton;
	// Embedded volume sliders on the options page (added in the Blueprint).
	UPROPERTY(meta = (BindWidgetOptional)) UAudioOptionsWidget* AudioOptions;

private:
	UFUNCTION() void OnContinueClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnOptionsBackClicked();

	// Rebuilds the navigable button set from the currently shown switcher page.
	void BuildNav();
};
