// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "ServerRowWidget.h"
#include "AudioOptionsWidget.h"
#include "OnlineSessionSubsystem.h"
#include "Base/KUPButton.h"
#include "Components/Widget.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/GameMapSettings.h"

UMainMenuWidget::UMainMenuWidget()
{
	// Receive "Back" actions so sub-panels can return to the root (see NativeOnHandleBackAction).
	bIsBackHandler = true;
}

UOnlineSessionSubsystem* UMainMenuWidget::GetOnline() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UOnlineSessionSubsystem>();
	}
	return nullptr;
}

FString UMainMenuWidget::GetToken() const
{
	return TokenInput ? TokenInput->GetText().ToString() : FString();
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind every button. The three "Back" buttons share a single handler. CommonUI buttons expose a
	// native OnClicked() event (not the dynamic UButton delegate), so bind with AddUObject.
	if (SoloButton)            SoloButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnSoloClicked);
	if (MultiplayerButton)     MultiplayerButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnMultiplayerClicked);
	if (OptionsButton)         OptionsButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnOptionsClicked);
	if (QuitButton)            QuitButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnQuitClicked);
	if (CreateButton)          CreateButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnCreateClicked);
	if (JoinButton)            JoinButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnJoinClicked);
	if (RefreshButton)         RefreshButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnRefreshClicked);
	if (MultiplayerBackButton) MultiplayerBackButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnBackToRootClicked);
	if (OptionsBackButton)     OptionsBackButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnBackToRootClicked);
	if (BrowserBackButton)     BrowserBackButton->OnClicked().AddUObject(this, &UMainMenuWidget::OnBackToRootClicked);

	// Listen to the online subsystem so the UI reacts to async EOS results.
	if (UOnlineSessionSubsystem* Online = GetOnline())
	{
		Online->OnLoginDone.AddDynamic(this, &UMainMenuWidget::HandleLoginDone);
		Online->OnHostDone.AddDynamic(this, &UMainMenuWidget::HandleHostDone);
		Online->OnFindDone.AddDynamic(this, &UMainMenuWidget::HandleFindDone);
		Online->OnJoinDone.AddDynamic(this, &UMainMenuWidget::HandleJoinDone);
	}

	ShowPanel(EMenuPanel::Root);
}

void UMainMenuWidget::ShowPanel(EMenuPanel Panel)
{
	CurrentPanel = Panel;
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(Panel));
	}

	// CommonUI handles navigation between focusable buttons; we just move focus to the panel's first.
	if (UWidget* Focus = GetPanelFocusTarget(Panel))
	{
		Focus->SetFocus();
	}
}

UWidget* UMainMenuWidget::GetPanelFocusTarget(EMenuPanel Panel) const
{
	switch (Panel)
	{
	case EMenuPanel::Root:        return SoloButton;
	case EMenuPanel::Multiplayer: return CreateButton;
	case EMenuPanel::Options:     return OptionsBackButton;
	case EMenuPanel::Browser:     return RefreshButton;
	default:                      return SoloButton;
	}
}

UWidget* UMainMenuWidget::NativeGetDesiredFocusTarget() const
{
	return GetPanelFocusTarget(CurrentPanel);
}

bool UMainMenuWidget::NativeOnHandleBackAction()
{
	// Any sub-panel goes back to the root list; the root itself does nothing. Always consume Back so
	// the activatable stack never pops the main menu out from under the player.
	if (CurrentPanel != EMenuPanel::Root)
	{
		ShowPanel(EMenuPanel::Root);
	}
	return true;
}

// ---------------------------------------------------------------- Root buttons
void UMainMenuWidget::OnSoloClicked()
{
	// Solo: a plain standalone level load — no EOS session required.
	const UGameMapSettings* MapSettings = GetDefault<UGameMapSettings>();
	UGameplayStatics::OpenLevel(this, FName(*MapSettings->GetGameplayMapName()));
}

void UMainMenuWidget::OnMultiplayerClicked() { ShowPanel(EMenuPanel::Multiplayer); }
void UMainMenuWidget::OnOptionsClicked()     { ShowPanel(EMenuPanel::Options); }
void UMainMenuWidget::OnBackToRootClicked()  { ShowPanel(EMenuPanel::Root); }

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// ---------------------------------------------------------------- Multiplayer
void UMainMenuWidget::OnCreateClicked()
{
	UOnlineSessionSubsystem* Online = GetOnline();
	if (!Online) return;

	if (GetToken().IsEmpty())
	{
		ShowPanel(EMenuPanel::Multiplayer);
		return;
	}

	// EOS requires a login before hosting; defer the host until login completes.
	if (!Online->IsLoggedIn())
	{
		Pending = EPendingOnline::Host;
		Online->Login(GetToken());
	}
	else
	{
		StartHost();
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	ShowPanel(EMenuPanel::Browser);

	UOnlineSessionSubsystem* Online = GetOnline();
	if (!Online) return;

	if (GetToken().IsEmpty())
	{
		SetBrowserStatus(TEXT("Entre un identifiant (token) dans le champ avant de rejoindre."));
		return;
	}

	// Same deferred pattern as hosting: login first, then search.
	if (!Online->IsLoggedIn())
	{
		Pending = EPendingOnline::Find;
		SetBrowserStatus(TEXT("Connexion EOS..."));
		Online->Login(GetToken());
	}
	else
	{
		StartFind();
	}
}

void UMainMenuWidget::OnRefreshClicked()
{
	if (UOnlineSessionSubsystem* Online = GetOnline())
	{
		if (Online->IsLoggedIn())
		{
			StartFind();
		}
	}
}

void UMainMenuWidget::StartHost()
{
	if (UOnlineSessionSubsystem* Online = GetOnline())
	{
		// The subsystem creates the session then ServerTravels to the lobby map.
		Online->HostGame(GetDefault<UGameMapSettings>()->GetLobbyMapName(), MaxPlayers);
	}
}

void UMainMenuWidget::StartFind()
{
	SetBrowserStatus(TEXT("Recherche de parties..."));
	if (UOnlineSessionSubsystem* Online = GetOnline())
	{
		Online->FindGames();
	}
}

// ---------------------------------------------------------------- Online callbacks
void UMainMenuWidget::HandleLoginDone(bool bSuccess)
{
	if (!bSuccess)
	{
		Pending = EPendingOnline::None;
		SetBrowserStatus(TEXT("Login EOS échoué. Vérifie le Dev Auth Tool et le token."));
		return;
	}

	// Run whatever action was waiting on the login.
	const EPendingOnline ToRun = Pending;
	Pending = EPendingOnline::None;
	if (ToRun == EPendingOnline::Host)      StartHost();
	else if (ToRun == EPendingOnline::Find) StartFind();
}

void UMainMenuWidget::HandleHostDone(bool bSuccess)
{
	// On success the subsystem ServerTravels to the lobby; nothing else to do here.
	if (!bSuccess)
	{
		SetBrowserStatus(TEXT("Échec de la création de la partie."));
	}
}

void UMainMenuWidget::HandleFindDone(int32 NumResults)
{
	if (!ServerListBox) return;
	ServerListBox->ClearChildren();

	UOnlineSessionSubsystem* Online = GetOnline();
	if (!Online) return;

	// One row widget per found session; the row stores its search index for joining.
	const TArray<FGameSessionInfo>& Sessions = Online->GetFoundSessions();
	for (int32 i = 0; i < Sessions.Num(); ++i)
	{
		if (!ServerRowClass) break;
		if (UServerRowWidget* Row = CreateWidget<UServerRowWidget>(this, ServerRowClass))
		{
			Row->Setup(Sessions[i], i);
			ServerListBox->AddChild(Row);
		}
	}

	SetBrowserStatus(FString::Printf(TEXT("%d partie(s) trouvée(s)."), NumResults));
}

void UMainMenuWidget::HandleJoinDone(bool bSuccess)
{
	// On success the subsystem ClientTravels into the host's lobby; nothing to do here.
	if (!bSuccess)
	{
		SetBrowserStatus(TEXT("Impossible de rejoindre la partie."));
	}
}

void UMainMenuWidget::SetBrowserStatus(const FString& Msg)
{
	if (BrowserStatusText)
	{
		BrowserStatusText->SetText(FText::FromString(Msg));
	}
}
