// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/KUPActivatableWidget.h"
#include "InGameMenuWidget.generated.h"

class UKUPButton;
class UWidgetSwitcher;
class UWidget;
class UAudioOptionsWidget;

/**
 * In-game pause/options screen (CommonUI activatable, solo and multiplayer). "Continue" closes the
 * menu (and unpauses in solo); the other buttons leave to the main menu or quit. Pausing itself is
 * handled by the owning player controller based on net mode. Back closes the options page if open,
 * otherwise the whole menu.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UInGameMenuWidget : public UKUPActivatableWidget
{
	GENERATED_BODY()

public:
	UInGameMenuWidget();

protected:
	virtual void NativeConstruct() override;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;

	UPROPERTY(meta = (BindWidget)) UKUPButton* ContinueButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* OptionsButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* MainMenuButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* QuitButton;

	// Optional: switch between the main button list (0) and the options panel (1).
	UPROPERTY(meta = (BindWidgetOptional)) UWidgetSwitcher* MenuSwitcher;
	UPROPERTY(meta = (BindWidgetOptional)) UKUPButton* OptionsBackButton;
	// Embedded volume sliders on the options page (added in the Blueprint).
	UPROPERTY(meta = (BindWidgetOptional)) UAudioOptionsWidget* AudioOptions;

private:
	UFUNCTION() void OnContinueClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnOptionsBackClicked();

	bool IsOnOptionsPage() const;
};
