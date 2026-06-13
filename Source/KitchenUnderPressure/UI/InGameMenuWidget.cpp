// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenuWidget.h"
#include "AudioOptionsWidget.h"
#include "Player/KitchenUnderPressurePlayerController.h"
#include "OnlineSessionSubsystem.h"
#include "Base/KUPButton.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/GameMapSettings.h"

UInGameMenuWidget::UInGameMenuWidget()
{
	// Back closes the options page or the whole menu (see NativeOnHandleBackAction).
	bIsBackHandler = true;
}

void UInGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)    ContinueButton->OnClicked().AddUObject(this, &UInGameMenuWidget::OnContinueClicked);
	if (OptionsButton)     OptionsButton->OnClicked().AddUObject(this, &UInGameMenuWidget::OnOptionsClicked);
	if (MainMenuButton)    MainMenuButton->OnClicked().AddUObject(this, &UInGameMenuWidget::OnMainMenuClicked);
	if (QuitButton)        QuitButton->OnClicked().AddUObject(this, &UInGameMenuWidget::OnQuitClicked);
	if (OptionsBackButton) OptionsBackButton->OnClicked().AddUObject(this, &UInGameMenuWidget::OnOptionsBackClicked);

	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
}

bool UInGameMenuWidget::IsOnOptionsPage() const
{
	return MenuSwitcher && MenuSwitcher->GetActiveWidgetIndex() == 1;
}

UWidget* UInGameMenuWidget::NativeGetDesiredFocusTarget() const
{
	return IsOnOptionsPage() ? static_cast<UWidget*>(OptionsBackButton) : static_cast<UWidget*>(ContinueButton);
}

bool UInGameMenuWidget::NativeOnHandleBackAction()
{
	if (IsOnOptionsPage())
	{
		OnOptionsBackClicked();
	}
	else
	{
		OnContinueClicked();
	}
	return true;
}

void UInGameMenuWidget::OnContinueClicked()
{
	// The controller owns the pause/input state, so it performs the actual close.
	if (AKitchenUnderPressurePlayerController* PC = Cast<AKitchenUnderPressurePlayerController>(GetOwningPlayer()))
	{
		PC->CloseInGameMenu();
	}
}

void UInGameMenuWidget::OnOptionsClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(1);
	if (OptionsBackButton) OptionsBackButton->SetFocus();
}

void UInGameMenuWidget::OnOptionsBackClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
	if (ContinueButton) ContinueButton->SetFocus();
}

void UInGameMenuWidget::OnMainMenuClicked()
{
	// Make sure we are not paused before any travel (solo freezes the world).
	UGameplayStatics::SetGamePaused(this, false);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* Online = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			// Leave the session first; the subsystem returns to the menu once it is actually
			// destroyed, so EOS does not keep us registered. In solo there is no session, so
			// it travels immediately.
			Online->LeaveGame(/*bReturnToMenu=*/true);
			return;
		}
	}

	const UGameMapSettings* MapSettings = GetDefault<UGameMapSettings>();
	UGameplayStatics::OpenLevel(this, FName(*MapSettings->GetMainMenuMapName()));
}

void UInGameMenuWidget::OnQuitClicked()
{
	// Best-effort: ask EOS to drop the session before the process exits.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* Online = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			Online->LeaveGame(/*bReturnToMenu=*/false);
		}
	}

	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
