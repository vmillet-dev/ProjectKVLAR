// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuNavWidget.generated.h"

class UButton;
class UWidget;
class USlider;
class UMenuInputSubsystem;
class UGameAudioSubsystem;
enum class EMenuInputDevice : uint8;
enum class EMenuSound : uint8;

/**
 * Base class for menus that can be driven with a gamepad or the keyboard. Holds an ordered
 * list of focusable items (buttons and sliders), the current index, and applies a colour-tint
 * highlight to the focused item while the active device is not the mouse. Subclasses fill the
 * list per panel via SetNavButtons()/SetNavWidgets() and override HandleBack(). Navigation,
 * validation and back also play the shared UI sounds through UGameAudioSubsystem.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UMenuNavWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Entry points called by UMenuInputComponent when the menu IMC fires.
	void Navigate(float Dir);
	void Accept();
	void Back();

	// Left/right adjustment of the focused slider (no-op when a button is focused).
	void Adjust(float Dir);

	// Re-apply the highlight for the current device (called when the menu becomes active).
	void RefreshFromDevice();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Subclasses define what "back" does for the visible panel (pop panel, close, leave...).
	virtual void HandleBack() {}

	// Replace the navigable set. SetNavButtons forwards to SetNavWidgets so existing
	// button-only callers keep working unchanged.
	void SetNavButtons(const TArray<UButton*>& Buttons);
	void SetNavWidgets(const TArray<UWidget*>& Widgets);

	// Recompute every item's tint from the current device + focused index.
	void ApplyHighlight();

	UPROPERTY(EditAnywhere, Category = "Menu|Navigation")
	FLinearColor HighlightColor = FLinearColor(1.f, 0.85f, 0.2f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Menu|Navigation")
	FLinearColor NormalColor = FLinearColor::White;

	// Step applied to a focused slider per left/right press.
	UPROPERTY(EditAnywhere, Category = "Menu|Navigation", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SliderStep = 0.05f;

private:
	bool IsNavigable(const UWidget* Item) const;
	void StepToFirstNavigable();
	void ValidateIndex();
	void ScrollCurrentIntoView();
	void HandleDeviceChanged(EMenuInputDevice Device);
	UMenuInputSubsystem* GetInputSub() const;

	// Audio helpers.
	UGameAudioSubsystem* GetGameAudio() const;
	void PlayMenuSound(EMenuSound Sound) const;

	// Mouse feedback: hover plays the navigate sound, click plays the accept sound.
	UFUNCTION() void HandleNavHover();
	UFUNCTION() void HandleNavClick();

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> NavItems;

	int32 CurrentIndex = INDEX_NONE;
	FDelegateHandle DeviceChangedHandle;
};
