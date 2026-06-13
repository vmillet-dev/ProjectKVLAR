// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuNavWidget.h"
#include "Core/MenuInputSubsystem.h"
#include "Subsystem/GameAudioSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ScrollBox.h"
#include "Engine/GameInstance.h"

void UMenuNavWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMenuInputSubsystem* Sub = GetInputSub())
	{
		DeviceChangedHandle = Sub->OnDeviceChanged.AddUObject(this, &UMenuNavWidget::HandleDeviceChanged);
	}
}

void UMenuNavWidget::NativeDestruct()
{
	if (UMenuInputSubsystem* Sub = GetInputSub())
	{
		Sub->OnDeviceChanged.Remove(DeviceChangedHandle);
	}
	DeviceChangedHandle.Reset();

	Super::NativeDestruct();
}

UMenuInputSubsystem* UMenuNavWidget::GetInputSub() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UMenuInputSubsystem>();
	}
	return nullptr;
}

UGameAudioSubsystem* UMenuNavWidget::GetGameAudio() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UGameAudioSubsystem>();
	}
	return nullptr;
}

void UMenuNavWidget::PlayMenuSound(EMenuSound Sound) const
{
	if (UGameAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->PlayMenuSound(Sound);
	}
}

void UMenuNavWidget::SetNavButtons(const TArray<UButton*>& Buttons)
{
	TArray<UWidget*> Widgets;
	Widgets.Reserve(Buttons.Num());
	for (UButton* Button : Buttons)
	{
		Widgets.Add(Button);
	}
	SetNavWidgets(Widgets);
}

void UMenuNavWidget::SetNavWidgets(const TArray<UWidget*>& Widgets)
{
	NavItems.Reset();
	for (UWidget* Widget : Widgets)
	{
		if (!Widget)
		{
			continue;
		}
		NavItems.Add(Widget);

		// Mouse feedback. AddUnique avoids stacking duplicates across panel rebuilds; the
		// click hook also fires when Accept() broadcasts OnClicked, so gamepad validation is
		// covered by the same path.
		if (UButton* Button = Cast<UButton>(Widget))
		{
			Button->OnHovered.AddUniqueDynamic(this, &UMenuNavWidget::HandleNavHover);
			Button->OnClicked.AddUniqueDynamic(this, &UMenuNavWidget::HandleNavClick);
		}
	}

	CurrentIndex = INDEX_NONE;
	StepToFirstNavigable();
	ApplyHighlight();
}

bool UMenuNavWidget::IsNavigable(const UWidget* Item) const
{
	return IsValid(Item) && Item->IsVisible() && Item->GetIsEnabled();
}

void UMenuNavWidget::StepToFirstNavigable()
{
	for (int32 i = 0; i < NavItems.Num(); ++i)
	{
		if (IsNavigable(NavItems[i]))
		{
			CurrentIndex = i;
			return;
		}
	}
	CurrentIndex = INDEX_NONE;
}

void UMenuNavWidget::ValidateIndex()
{
	if (!NavItems.IsValidIndex(CurrentIndex) || !IsNavigable(NavItems[CurrentIndex]))
	{
		StepToFirstNavigable();
	}
}

void UMenuNavWidget::Navigate(float Dir)
{
	if (FMath::IsNearlyZero(Dir) || NavItems.Num() == 0)
	{
		return;
	}

	// Dir > 0 = up (previous index), Dir < 0 = down (next index).
	const int32 Step = (Dir > 0.f) ? -1 : 1;
	const int32 Count = NavItems.Num();

	int32 Index = (CurrentIndex == INDEX_NONE) ? (Step > 0 ? -1 : Count) : CurrentIndex;
	for (int32 Tries = 0; Tries < Count; ++Tries)
	{
		Index = (Index + Step + Count) % Count;
		if (IsNavigable(NavItems[Index]))
		{
			CurrentIndex = Index;
			ApplyHighlight();
			ScrollCurrentIntoView();
			PlayMenuSound(EMenuSound::Navigate);
			return;
		}
	}
}

void UMenuNavWidget::Adjust(float Dir)
{
	if (FMath::IsNearlyZero(Dir) || !NavItems.IsValidIndex(CurrentIndex))
	{
		return;
	}

	USlider* Slider = Cast<USlider>(NavItems[CurrentIndex]);
	if (!Slider || !IsNavigable(Slider))
	{
		return;
	}

	const float Old = Slider->GetValue();
	const float NewValue = FMath::Clamp(Old + (Dir > 0.f ? SliderStep : -SliderStep), 0.f, 1.f);
	if (FMath::IsNearlyEqual(NewValue, Old))
	{
		return;
	}

	// SetValue alone does not notify listeners, so broadcast to drive the options widget.
	Slider->SetValue(NewValue);
	Slider->OnValueChanged.Broadcast(NewValue);
	PlayMenuSound(EMenuSound::Navigate);

	// Persist once per discrete gamepad/keyboard step (no disk spam like a mouse drag).
	if (UGameAudioSubsystem* Audio = GetGameAudio())
	{
		Audio->SaveSettings();
	}
}

void UMenuNavWidget::Accept()
{
	const UMenuInputSubsystem* Sub = GetInputSub();
	if (Sub && !Sub->IsHighlightActive())
	{
		// Mouse mode: validation happens through actual clicks, not this action.
		return;
	}

	if (NavItems.IsValidIndex(CurrentIndex) && IsNavigable(NavItems[CurrentIndex]))
	{
		// The accept sound rides on the OnClicked broadcast (see HandleNavClick).
		if (UButton* Button = Cast<UButton>(NavItems[CurrentIndex]))
		{
			Button->OnClicked.Broadcast();
		}
		// A focused slider is changed with Adjust(), so Accept does nothing for it.
	}
}

void UMenuNavWidget::Back()
{
	PlayMenuSound(EMenuSound::Back);
	HandleBack();
}

void UMenuNavWidget::RefreshFromDevice()
{
	if (CurrentIndex == INDEX_NONE)
	{
		StepToFirstNavigable();
	}
	ApplyHighlight();
}

void UMenuNavWidget::ApplyHighlight()
{
	ValidateIndex();

	const UMenuInputSubsystem* Sub = GetInputSub();
	const bool bHighlightActive = Sub ? Sub->IsHighlightActive() : false;

	for (int32 i = 0; i < NavItems.Num(); ++i)
	{
		UWidget* Item = NavItems[i];
		if (!Item)
		{
			continue;
		}
		const bool bFocused = bHighlightActive && i == CurrentIndex && IsNavigable(Item);
		const FLinearColor Color = bFocused ? HighlightColor : NormalColor;

		if (UButton* Button = Cast<UButton>(Item))
		{
			Button->SetBackgroundColor(Color);
		}
		else if (USlider* Slider = Cast<USlider>(Item))
		{
			Slider->SetSliderHandleColor(Color);
		}
	}
}

void UMenuNavWidget::HandleDeviceChanged(EMenuInputDevice Device)
{
	if (CurrentIndex == INDEX_NONE)
	{
		StepToFirstNavigable();
	}
	ApplyHighlight();
}

void UMenuNavWidget::HandleNavHover()
{
	// Mouse only: in gamepad/keyboard mode the highlight is active and Navigate() already
	// plays the sound, so guard against a double blip.
	const UMenuInputSubsystem* Sub = GetInputSub();
	if (!Sub || !Sub->IsHighlightActive())
	{
		PlayMenuSound(EMenuSound::Navigate);
	}
}

void UMenuNavWidget::HandleNavClick()
{
	PlayMenuSound(EMenuSound::Accept);
}

void UMenuNavWidget::ScrollCurrentIntoView()
{
	if (!NavItems.IsValidIndex(CurrentIndex) || !NavItems[CurrentIndex])
	{
		return;
	}

	// Walk up to a containing ScrollBox (if any) and keep the focused item visible.
	for (UWidget* Parent = NavItems[CurrentIndex]->GetParent(); Parent; Parent = Parent->GetParent())
	{
		if (UScrollBox* Scroll = Cast<UScrollBox>(Parent))
		{
			Scroll->ScrollWidgetIntoView(NavItems[CurrentIndex], true);
			return;
		}
	}
}
