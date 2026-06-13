// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioOptionsWidget.generated.h"

class UAnalogSlider;
class UTextBlock;
class UAudioSubsystem;

/**
 * Reusable options panel holding the Master / Music / SFX volume sliders. Embedded in both the
 * main-menu and the in-game pause screens. The sliders apply their value live to UAudioSubsystem;
 * UAnalogSlider makes them adjustable with the gamepad when focused, and CommonUI handles navigating
 * to them, so no manual nav wiring is needed. Volumes persist on mouse release and on panel close.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAudioOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget)) UAnalogSlider* MasterSlider;
	UPROPERTY(meta = (BindWidget)) UAnalogSlider* MusicSlider;
	UPROPERTY(meta = (BindWidget)) UAnalogSlider* SfxSlider;

	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MasterValueText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MusicValueText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* SfxValueText;

private:
	UFUNCTION() void OnMasterChanged(float Value);
	UFUNCTION() void OnMusicChanged(float Value);
	UFUNCTION() void OnSfxChanged(float Value);

	// Mouse-drag finished on any slider: write the volumes to disk once.
	UFUNCTION() void OnSliderReleased();

	void InitSlider(UAnalogSlider* Slider, float Value, UTextBlock* ValueText);
	void UpdateValueText(UTextBlock* Text, float Value) const;
	UAudioSubsystem* GetGameAudio() const;
};
