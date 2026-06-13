// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "LobbyPlayerController.h"
#include "LobbyPlayerState.h"
#include "Base/KUPButton.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"

ULobbyWidget::ULobbyWidget()
{
	// Back leaves the lobby (see NativeOnHandleBackAction).
	bIsBackHandler = true;
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton) ReadyButton->OnClicked().AddUObject(this, &ULobbyWidget::OnReadyClicked);
	if (StartButton) StartButton->OnClicked().AddUObject(this, &ULobbyWidget::OnStartClicked);
	if (LeaveButton) LeaveButton->OnClicked().AddUObject(this, &ULobbyWidget::OnLeaveClicked);

	// Poll the replicated player list on a timer rather than wiring per-property
	// replication callbacks — simple and more than enough for a small lobby.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimer, this, &ULobbyWidget::RefreshList, 0.3f, true);
	}

	RefreshList();
}

void ULobbyWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::NativeDestruct();
}

UWidget* ULobbyWidget::NativeGetDesiredFocusTarget() const
{
	return ReadyButton;
}

bool ULobbyWidget::NativeOnHandleBackAction()
{
	OnLeaveClicked();
	return true;
}

void ULobbyWidget::OnReadyClicked()
{
	bLocalReady = !bLocalReady;

	if (ALobbyPlayerController* PC = GetLobbyPC())
	{
		PC->ServerSetReady(bLocalReady);
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(bLocalReady ? TEXT("Tu es prêt.") : TEXT("En attente...")));
	}
}

void ULobbyWidget::OnStartClicked()
{
	if (ALobbyPlayerController* PC = GetLobbyPC())
	{
		PC->ServerRequestStart();
	}
}

void ULobbyWidget::OnLeaveClicked()
{
	if (ALobbyPlayerController* PC = GetLobbyPC())
	{
		PC->LeaveLobby();
	}
}

void ULobbyWidget::RefreshList()
{
	if (!PlayerListBox) return;
	PlayerListBox->ClearChildren();

	const UWorld* World = GetWorld();
	const AGameStateBase* GS = World ? World->GetGameState() : nullptr;
	if (!GS) return;

	int32 ReadyCount = 0;
	for (const APlayerState* PS : GS->PlayerArray)
	{
		const ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS);
		if (!LobbyPS) continue;

		const bool bReady = LobbyPS->bIsReady;
		if (bReady) ++ReadyCount;

		// Build a simple text line per player without needing a dedicated row widget.
		UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		const FString Name = LobbyPS->GetPlayerName();
		Line->SetText(FText::FromString(FString::Printf(TEXT("%s  -  %s"),
			*Name, bReady ? TEXT("Prêt") : TEXT("Pas prêt"))));
		PlayerListBox->AddChild(Line);
	}

	const int32 Num = GS->PlayerArray.Num();
	if (CountText)
	{
		CountText->SetText(FText::FromString(
			FString::Printf(TEXT("Joueurs : %d  (prêts : %d)"), Num, ReadyCount)));
	}

	// Only the host may start, and only when every connected player is ready. CommonUI keeps focus
	// valid as Start's enabled/visible state changes.
	if (StartButton)
	{
		StartButton->SetVisibility(IsHost() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StartButton->SetIsEnabled(IsHost() && Num > 0 && ReadyCount == Num);
	}
}

ALobbyPlayerController* ULobbyWidget::GetLobbyPC() const
{
	return Cast<ALobbyPlayerController>(GetOwningPlayer());
}

bool ULobbyWidget::IsHost() const
{
	// On a listen server the local net mode is ListenServer/Standalone; pure clients
	// are NM_Client. Only the host should see the Start button.
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client;
}
