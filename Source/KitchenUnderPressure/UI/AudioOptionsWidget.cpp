// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioOptionsWidget.h"
#include "Subsystem/GameAudioSubsystem.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UAudioOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UGameAudioSubsystem* Audio = GetGameAudio();
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

void UAudioOptionsWidget::InitSlider(USlider* Slider, float Value, UTextBlock* ValueText)
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
	if (UGameAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetMasterVolume(Value);
	}
	UpdateValueText(MasterValueText, Value);
}

void UAudioOptionsWidget::OnMusicChanged(float Value)
{
	if (UGameAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetMusicVolume(Value);
	}
	UpdateValueText(MusicValueText, Value);
}

void UAudioOptionsWidget::OnSfxChanged(float Value)
{
	if (UGameAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SetSfxVolume(Value);
	}
	UpdateValueText(SfxValueText, Value);
}

void UAudioOptionsWidget::OnSliderReleased()
{
	if (UGameAudioSubsystem* Audio = GetGameAudio())
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

TArray<UWidget*> UAudioOptionsWidget::GetNavWidgets() const
{
	TArray<UWidget*> Widgets;
	if (MasterSlider) { Widgets.Add(MasterSlider); }
	if (MusicSlider)  { Widgets.Add(MusicSlider); }
	if (SfxSlider)    { Widgets.Add(SfxSlider); }
	return Widgets;
}

UGameAudioSubsystem* UAudioOptionsWidget::GetGameAudio() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameAudioSubsystem>() : nullptr;
}
