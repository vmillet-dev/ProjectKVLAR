// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuNavWidget.h"
#include "LobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class ALobbyPlayerController;

/**
 * Waiting-room UI. Periodically rebuilds the player list from the replicated
 * PlayerArray, toggles the local ready flag, and lets the host start the match.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ULobbyWidget : public UMenuNavWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// "Back" leaves the lobby and returns to the main menu.
	virtual void HandleBack() override;

	UPROPERTY(meta = (BindWidget)) UScrollBox* PlayerListBox;
	UPROPERTY(meta = (BindWidget)) UButton* ReadyButton;
	UPROPERTY(meta = (BindWidget)) UButton* StartButton;
	UPROPERTY(meta = (BindWidget)) UButton* LeaveButton;
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
