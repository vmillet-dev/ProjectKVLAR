// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "KUPActivatableWidget.generated.h"

/**
 * Project base for full-screen menus pushed onto the primary layout's activatable stack. CommonUI
 * already handles activation, focus, gamepad/keyboard/mouse routing and the Back action; concrete
 * screens override NativeGetDesiredFocusTarget() to pick their first focus target and configure the
 * Back behaviour in the Blueprint. Kept as a thin base so shared menu behaviour can hang off it later.
 */
UCLASS(Abstract, Blueprintable)
class KITCHENUNDERPRESSURE_API UKUPActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
};
