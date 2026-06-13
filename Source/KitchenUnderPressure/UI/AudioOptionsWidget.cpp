// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioOptionsWidget.h"
#include "Audio/AudioSubsystem.h"
#include "AnalogSlider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UAudioOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UAudioSubsystem* Audio = GetGameAudio();
	const float Master = Audio ? Audio->GetMasterVolume() : 1.f;
	const float Music = Audio ? Audio->GetMusicVolume() : 1.f;
	const float Sfx = Audio ? Audio->GetSfxVolume() : 1.f;

	// Set the value before binding so the initial sync does not fire the change handlers.
	InitSlider(MasterSlider, Master, MasterValueText);
	InitSlider(MusicSlider, Music, MusicValueText);
	InitSlider(SfxSlider, Sfx, SfxValueText);

	if (MasterSlider)
	{
		MasterSlider->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnMasterChanged);
		MasterSlider->OnMouseCaptureEnd.AddDynamic(this, &UAudioOptionsWidget::OnSliderReleased);
	}
	if (MusicSlider)
	{
		MusicSlider->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnMusicChanged);
		MusicSlider->OnMouseCaptureEnd.AddDynamic(this, &UAudioOptionsWidget::OnSliderReleased);
	}
	if (SfxSlider)
	{
		SfxSlider->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnSfxChanged);
		SfxSlider->OnMouseCaptureEnd.AddDynamic(this, &UAudioOptionsWidget::OnSliderReleased);
	}
}

void UAudioOptionsWidget::NativeDestruct()
{
	// Persist on close so gamepad/keyboard adjustments (which don't fire OnMouseCaptureEnd) are saved.
	if (UAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SaveSettings();
	}
	Super::NativeDestruct();
}

void UAudioOptionsWidget::InitSlider(UAnalogSlider* Slider, float Value, UTextBlock* ValueText)
{
	if (Slider)
	{
		Slider->SetMinValue(0.f);
		Slider->SetMaxValue(1.f);
		Slider->SetValue(Value);
	}
	UpdateValueText(ValueText, Value);
}

void UAudioOptionsWidget::OnMasterChanged(float Value)
{
	if (UAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetMasterVolume(Value);
	}
	UpdateValueText(MasterValueText, Value);
}

void UAudioOptionsWidget::OnMusicChanged(float Value)
{
	if (UAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetMusicVolume(Value);
	}
	UpdateValueText(MusicValueText, Value);
}

void UAudioOptionsWidget::OnSfxChanged(float Value)
{
	if (UAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetSfxVolume(Value);
	}
	UpdateValueText(SfxValueText, Value);
}

void UAudioOptionsWidget::OnSliderReleased()
{
	if (UAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SaveSettings();
	}
}

void UAudioOptionsWidget::UpdateValueText(UTextBlock* Text, float Value) const
{
	if (Text)
	{
		Text->SetText(FText::AsPercent(Value));
	}
}

UAudioSubsystem* UAudioOptionsWidget::GetGameAudio() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UAudioSubsystem>() : nullptr;
}
