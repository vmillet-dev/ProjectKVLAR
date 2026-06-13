// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SubclassOf.h"
#include "KUPUISubsystem.generated.h"

class UKUPPrimaryLayout;
class UCommonActivatableWidget;

/**
 * Per-local-player UI router. Lazily creates the primary layout (UKUPUISettings::PrimaryLayout) once,
 * adds it to the viewport, and pushes/pops screens onto its menu stack. Controllers call PushScreen()
 * instead of CreateWidget + AddToViewport, so CommonUI owns focus, input routing and Back navigation.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UKUPUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// Create the layout if needed and push a screen onto its menu stack (null on failure).
	UCommonActivatableWidget* PushScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass);

	// Remove every pushed screen (the layout stays in the viewport).
	void ClearScreens();

private:
	UKUPPrimaryLayout* EnsureLayout();

	UPROPERTY()
	TObjectPtr<UKUPPrimaryLayout> Layout;
};
