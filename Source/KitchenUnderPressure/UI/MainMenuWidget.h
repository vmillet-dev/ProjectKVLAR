// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuNavWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UScrollBox;
class UWidgetSwitcher;
class UServerRowWidget;
class UOnlineSessionSubsystem;
class UAudioOptionsWidget;

/**
 * Root main-menu widget. Drives top-level navigation (Solo / Multiplayer / Options /
 * Quit) through an internal WidgetSwitcher, handles the EOS login + host/find flow,
 * and builds the joinable-session list from one UServerRowWidget per result.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UMainMenuWidget : public UMenuNavWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// "Back" returns to the root panel from any sub-panel.
	virtual void HandleBack() override;

	// --- Root panel ---
	UPROPERTY(meta = (BindWidget)) UButton* SoloButton;
	UPROPERTY(meta = (BindWidget)) UButton* MultiplayerButton;
	UPROPERTY(meta = (BindWidget)) UButton* OptionsButton;
	UPROPERTY(meta = (BindWidget)) UButton* QuitButton;

	// --- Multiplayer panel ---
	UPROPERTY(meta = (BindWidget)) UButton* CreateButton;
	UPROPERTY(meta = (BindWidget)) UButton* JoinButton;
	UPROPERTY(meta = (BindWidget)) UButton* MultiplayerBackButton;
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* TokenInput;

	// --- Options panel ---
	UPROPERTY(meta = (BindWidget)) UButton* OptionsBackButton;
	// Embedded volume sliders (added in the Blueprint). Optional so the menu still works
	// before the WBP is updated.
	UPROPERTY(meta = (BindWidgetOptional)) UAudioOptionsWidget* AudioOptions;

	// --- Server browser panel ---
	UPROPERTY(meta = (BindWidget)) UScrollBox* ServerListBox;
	UPROPERTY(meta = (BindWidget)) UButton* RefreshButton;
	UPROPERTY(meta = (BindWidget)) UButton* BrowserBackButton;
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

	// Currently visible panel, tracked so navigation can be rebuilt on async refreshes.
	EMenuPanel CurrentPanel = EMenuPanel::Root;

	// Online action to run once an async EOS login succeeds.
	enum class EPendingOnline : uint8 { None, Host, Find };
	EPendingOnline Pending = EPendingOnline::None;

	// Button handlers
	UFUNCTION() void OnSoloClicked();
	UFUNCTION() void OnMultiplayerClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnCreateClicked();
	UFUNCTION() void OnJoinClicked();
	UFUNCTION() void OnRefreshClicked();
	UFUNCTION() void OnBackToRootClicked();

	// Online subsystem delegate handlers
	UFUNCTION() void HandleLoginDone(bool bSuccess);
	UFUNCTION() void HandleHostDone(bool bSuccess);
	UFUNCTION() void HandleFindDone(int32 NumResults);
	UFUNCTION() void HandleJoinDone(bool bSuccess);

	void ShowPanel(EMenuPanel Panel);

	// Collects the navigable buttons for a panel and hands them to the base widget.
	void BuildNavForPanel(EMenuPanel Panel);

	void StartHost();
	void StartFind();
	void SetBrowserStatus(const FString& Msg);

	UOnlineSessionSubsystem* GetOnline() const;
	FString GetToken() const;
};
