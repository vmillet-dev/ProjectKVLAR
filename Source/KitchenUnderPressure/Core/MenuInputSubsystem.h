// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MenuInputSubsystem.generated.h"

// Which device last drove input. Highlight shows for everything except Mouse;
// the mouse cursor is hidden only for Gamepad.
enum class EMenuInputDevice : uint8
{
	Mouse,
	Keyboard,
	Gamepad
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMenuInputDeviceChanged, EMenuInputDevice);

class FMenuInputPreprocessor;
class FNavigationConfig;

/**
 * Persists across maps. Watches raw Slate input to know whether the player is currently
 * using the mouse, the keyboard or the gamepad, and broadcasts changes so menus can show
 * the focus highlight (non-mouse) and hide the cursor (gamepad). Also disables Slate's
 * built-in navigation so it does not fight the IMC-driven menu navigation.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UMenuInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	EMenuInputDevice GetDevice() const { return CurrentDevice; }

	// True when a focus highlight should be visible (gamepad or keyboard).
	bool IsHighlightActive() const { return CurrentDevice != EMenuInputDevice::Mouse; }

	// True when the OS mouse cursor should be shown (anything but gamepad).
	bool IsCursorVisible() const { return CurrentDevice != EMenuInputDevice::Gamepad; }

	FOnMenuInputDeviceChanged OnDeviceChanged;

	// Called by the input preprocessor; broadcasts only on an actual change.
	void ReportDevice(EMenuInputDevice NewDevice);

private:
	EMenuInputDevice CurrentDevice = EMenuInputDevice::Mouse;

	TSharedPtr<FMenuInputPreprocessor> InputProcessor;
	TSharedPtr<FNavigationConfig> PreviousNavConfig;
};
