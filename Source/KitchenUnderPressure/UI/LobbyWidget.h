// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/KUPActivatableWidget.h"
#include "LobbyWidget.generated.h"

class UKUPButton;
class UTextBlock;
class UScrollBox;
class UWidget;
class ALobbyPlayerController;

/**
 * Waiting-room screen (CommonUI activatable). Periodically rebuilds the player list from the
 * replicated PlayerArray, toggles the local ready flag, and lets the host start the match.
 * Back leaves the lobby and returns to the main menu.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ULobbyWidget : public UKUPActivatableWidget
{
	GENERATED_BODY()

public:
	ULobbyWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;

	UPROPERTY(meta = (BindWidget)) UScrollBox* PlayerListBox;
	UPROPERTY(meta = (BindWidget)) UKUPButton* ReadyButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* StartButton;
	UPROPERTY(meta = (BindWidget)) UKUPButton* LeaveButton;
	UPROPERTY(meta = (BindWidget)) UTextBlock* StatusText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* CountText;

private:
	UFUNCTION() void OnReadyClicked();
	UFUNCTION() void OnStartClicked();
	UFUNCTION() void OnLeaveClicked();

	// Rebuilds the player list from the game state's replicated PlayerArray.
	void RefreshList();

	ALobbyPlayerController* GetLobbyPC() const;
	bool IsHost() const;

	bool bLocalReady = false;
	FTimerHandle RefreshTimer;
};
