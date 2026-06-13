// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/KUPActivatableWidget.h"
#include "MainMenuWidget.generated.h"

class UKUPButton;
class UEditableTextBox;
class UTextBlock;
class UScrollBox;
class UWidgetSwitcher;
class UWidget;
class UServerRowWidget;
class UOnlineSessionSubsystem;
class UAudioOptionsWidget;

/**
 * Root main-menu screen (CommonUI activatable). Drives top-level navigation (Solo / Multiplayer /
 * Options / Quit) through an internal WidgetSwitcher, handles the EOS login + host/find flow, and
 * builds the joinable-session list from one UServerRowWidget per result. Focus and gamepad/keyboard/
 * mouse routing are handled by CommonUI; Back returns to the root panel.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UMainMenuWidget : public UKUPActivatableWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget();

protected:
	virtual void NativeConstruct() override;

	// Focus target for the active panel (gamepad / keyboard).
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// Back returns to the root panel from any sub-panel; always consumed so the menu is never popped.
	virtual bool NativeOnHandleBackAction() override;

	// --- Root panel ---
	UPROPERTY(meta = (BindWidget)) UKUPButton* SoloButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* MultiplayerButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* OptionsButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* QuitButton;

	// --- Multiplayer panel ---
	UPROPERTY(meta = (BindWidget)) UKUPButton* CreateButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* JoinButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* MultiplayerBackButton;
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* TokenInput;

	// --- Options panel ---
	UPROPERTY(meta = (BindWidget)) UKUPButton* OptionsBackButton;
	// Embedded volume sliders (added in the Blueprint). Optional so the menu still works before the WBP is updated.
	UPROPERTY(meta = (BindWidgetOptional)) UAudioOptionsWidget* AudioOptions;

	// --- Server browser panel ---
	UPROPERTY(meta = (BindWidget)) UScrollBox* ServerListBox;
	UPROPERTY(meta = (BindWidget)) UKUPButton* RefreshButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* BrowserBackButton;
	UPROPERTY(meta = (BindWidget)) UTextBlock* BrowserStatusText;

	// Switcher holding the 4 panels (indices defined by EMenuPanel).
	UPROPERTY(meta = (BindWidget)) UWidgetSwitcher* MenuSwitcher;

	// Widget class instantiated once per found session row.
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	TSubclassOf<UServerRowWidget> ServerRowClass;

	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	int32 MaxPlayers = 5;

private:
	// Panel indices inside MenuSwitcher (must match the child order in the Blueprint).
	enum class EMenuPanel : int32 { Root = 0, Multiplayer = 1, Options = 2, Browser = 3 };

	// Currently visible panel, tracked so Back and focus know where they are.
	EMenuPanel CurrentPanel = EMenuPanel::Root;

	// Online action to run once an async EOS login succeeds.
	enum class EPendingOnline : uint8 { None, Host, Find };
	EPendingOnline Pending = EPendingOnline::None;

	// Button handlers (bound to UCommonButtonBase::OnClicked()).
	UFUNCTION() void OnSoloClicked();
	UFUNCTION() void OnMultiplayerClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnCreateClicked();
	UFUNCTION() void OnJoinClicked();
	UFUNCTION() void OnRefreshClicked();
	UFUNCTION() void OnBackToRootClicked();

	// Online subsystem delegate handlers.
	UFUNCTION() void HandleLoginDone(bool bSuccess);
	UFUNCTION() void HandleHostDone(bool bSuccess);
	UFUNCTION() void HandleFindDone(int32 NumResults);
	UFUNCTION() void HandleJoinDone(bool bSuccess);

	void ShowPanel(EMenuPanel Panel);

	// First widget CommonUI should focus for the given panel.
	UWidget* GetPanelFocusTarget(EMenuPanel Panel) const;

	void StartHost();
	void StartFind();
	void SetBrowserStatus(const FString& Msg);

	UOnlineSessionSubsystem* GetOnline() const;
	FString GetToken() const;
};
