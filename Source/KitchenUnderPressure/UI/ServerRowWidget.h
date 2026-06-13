// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRowWidget.generated.h"

struct FGameSessionInfo;
class UKUPButton;
class UTextBlock;

/**
 * Single entry in the joinable-session list. Displays one found session and, when clicked, joins it
 * through the online subsystem using its stored search index. The join button is a UKUPButton, so
 * CommonUI navigates to it automatically when the browser is focused.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UServerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fills the row texts and remembers which search result it represents.
	void Setup(const FGameSessionInfo& Info, int32 Index);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) UTextBlock* OwnerText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* SlotsText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* PingText;
	UPROPERTY(meta = (BindWidget)) UKUPButton* JoinRowButton;

private:
	UFUNCTION() void OnJoinRowClicked();

	int32 SearchResultIndex = -1;
};
