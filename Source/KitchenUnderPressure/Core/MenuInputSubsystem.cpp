// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuInputSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/NavigationConfig.h"

// Slate input preprocessor: sees every mouse/keyboard/gamepad event before the rest of
// the UI. It never consumes input (always returns false); it only classifies the device.
class FMenuInputPreprocessor : public IInputProcessor
{
public:
	explicit FMenuInputPreprocessor(UMenuInputSubsystem* InOwner)
		: Owner(InOwner)
	{
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		Report(InKeyEvent.GetKey().IsGamepadKey() ? EMenuInputDevice::Gamepad : EMenuInputDevice::Keyboard);
		return false;
	}

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override
	{
		// Only treat actual gamepad sticks/triggers as gamepad activity, above a dead zone
		// so resting-stick drift does not flip the device away from mouse/keyboard.
		if (InAnalogInputEvent.GetKey().IsGamepadKey() && FMath::Abs(InAnalogInputEvent.GetAnalogValue()) > AnalogDeadZone)
		{
			Report(EMenuInputDevice::Gamepad);
		}
		return false;
	}

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetCursorDelta().SizeSquared() > MouseMoveThresholdSq)
		{
			Report(EMenuInputDevice::Mouse);
		}
		return false;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		Report(EMenuInputDevice::Mouse);
		return false;
	}

private:
	void Report(EMenuInputDevice Device)
	{
		if (Owner.IsValid())
		{
			Owner->ReportDevice(Device);
		}
	}

	TWeakObjectPtr<UMenuInputSubsystem> Owner;

	static constexpr float AnalogDeadZone = 0.2f;
	static constexpr float MouseMoveThresholdSq = 4.0f; // ~2 px, ignores sub-pixel jitter
};

void UMenuInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// No Slate on a dedicated server; nothing to watch there.
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	InputProcessor = MakeShared<FMenuInputPreprocessor>(this);
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);

	// Take over navigation: we drive it ourselves via the menu IMC, so Slate must not
	// also move focus / activate buttons on the same keys.
	PreviousNavConfig = FSlateApplication::Get().GetNavigationConfig();
	FSlateApplication::Get().SetNavigationConfig(MakeShared<FNullNavigationConfig>());
}

void UMenuInputSubsystem::Deinitialize()
{
	if (FSlateApplication::IsInitialized())
	{
		if (InputProcessor.IsValid())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		}
		if (PreviousNavConfig.IsValid())
		{
			FSlateApplication::Get().SetNavigationConfig(PreviousNavConfig.ToSharedRef());
		}
	}

	InputProcessor.Reset();
	PreviousNavConfig.Reset();

	Super::Deinitialize();
}

void UMenuInputSubsystem::ReportDevice(EMenuInputDevice NewDevice)
{
	if (NewDevice != CurrentDevice)
	{
		CurrentDevice = NewDevice;
		OnDeviceChanged.Broadcast(NewDevice);
	}
}
