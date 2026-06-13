// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenuWidget.h"
#include "AudioOptionsWidget.h"
#include "Core/KitchenUnderPressurePlayerController.h"
#include "OnlineSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/GameMapSettings.h"

void UInGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)    ContinueButton->OnClicked.AddDynamic(this, &UInGameMenuWidget::OnContinueClicked);
	if (OptionsButton)     OptionsButton->OnClicked.AddDynamic(this, &UInGameMenuWidget::OnOptionsClicked);
	if (MainMenuButton)    MainMenuButton->OnClicked.AddDynamic(this, &UInGameMenuWidget::OnMainMenuClicked);
	if (QuitButton)        QuitButton->OnClicked.AddDynamic(this, &UInGameMenuWidget::OnQuitClicked);
	if (OptionsBackButton) OptionsBackButton->OnClicked.AddDynamic(this, &UInGameMenuWidget::OnOptionsBackClicked);

	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);

	BuildNav();
}

void UInGameMenuWidget::BuildNav()
{
	// Switcher page 1 is the options panel; everything else is the main button list.
	const int32 Page = MenuSwitcher ? MenuSwitcher->GetActiveWidgetIndex() : 0;
	if (Page == 1)
	{
		// Sliders (if the WBP embeds them) then the Back button — a mixed widget list.
		TArray<UWidget*> Widgets;
		if (AudioOptions)
		{
			Widgets.Append(AudioOptions->GetNavWidgets());
		}
		Widgets.Add(OptionsBackButton);
		SetNavWidgets(Widgets);
	}
	else
	{
		SetNavButtons({ ContinueButton, OptionsButton, MainMenuButton, QuitButton });
	}
}

void UInGameMenuWidget::HandleBack()
{
	if (MenuSwitcher && MenuSwitcher->GetActiveWidgetIndex() == 1)
	{
		OnOptionsBackClicked();
	}
	else
	{
		OnContinueClicked();
	}
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
	BuildNav();
}

void UInGameMenuWidget::OnOptionsBackClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
	BuildNav();
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
