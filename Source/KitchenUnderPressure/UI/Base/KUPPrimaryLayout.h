// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Templates/SubclassOf.h"
#include "KUPPrimaryLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

/**
 * Root UI shell for a local player: a single widget added to the viewport once, holding the menu
 * layer (an activatable widget stack). Screens are pushed/popped on this stack by UKUPUISubsystem
 * rather than each one being added to the viewport directly, so back navigation and focus are
 * managed by CommonUI. The matching WBP must bind a CommonActivatableWidgetStack named "MenuStack".
 */
UCLASS(Abstract, Blueprintable)
class KITCHENUNDERPRESSURE_API UKUPPrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// Push a screen onto the menu layer; returns the created widget (null on failure).
	UCommonActivatableWidget* PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass);

	// Remove every screen from the menu layer.
	void ClearMenuStack();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
};
