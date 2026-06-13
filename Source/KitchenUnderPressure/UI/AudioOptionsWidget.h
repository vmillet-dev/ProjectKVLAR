// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioOptionsWidget.generated.h"

class USlider;
class UTextBlock;
class UWidget;
class UGameAudioSubsystem;

/**
 * Reusable options panel holding the Master / Music / SFX volume sliders. Embedded in both the
 * main-menu and the in-game pause menu. The sliders apply their value live to UGameAudioSubsystem
 * and persist on release. Gamepad/keyboard focus and left/right adjustment are driven by the
 * parent UMenuNavWidget, which fetches the sliders through GetNavWidgets().
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAudioOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Sliders to hand to the parent menu's navigation list (Master, Music, SFX order).
	TArray<UWidget*> GetNavWidgets() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) USlider* MasterSlider;
	UPROPERTY(meta = (BindWidget)) USlider* MusicSlider;
	UPROPERTY(meta = (BindWidget)) USlider* SfxSlider;

	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MasterValueText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MusicValueText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* SfxValueText;

private:
	UFUNCTION() void OnMasterChanged(float Value);
	UFUNCTION() void OnMusicChanged(float Value);
	UFUNCTION() void OnSfxChanged(float Value);

	// Mouse-drag finished on any slider: write the volumes to disk once.
	UFUNCTION() void OnSliderReleased();

	void InitSlider(USlider* Slider, float Value, UTextBlock* ValueText);
	void UpdateValueText(UTextBlock* Text, float Value) const;
	UGameAudioSubsystem* GetGameAudio() const;
};
