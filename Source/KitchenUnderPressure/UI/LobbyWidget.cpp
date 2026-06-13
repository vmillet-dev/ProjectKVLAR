// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "LobbyPlayerController.h"
#include "LobbyPlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton) ReadyButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnReadyClicked);
	if (StartButton) StartButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartClicked);
	if (LeaveButton) LeaveButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnLeaveClicked);

	// Start is skipped automatically while it is collapsed/disabled (non-host or not all ready).
	SetNavButtons({ ReadyButton, StartButton, LeaveButton });

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

void ULobbyWidget::HandleBack()
{
	OnLeaveClicked();
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

	// Only the host may start, and only when every connected player is ready.
	if (StartButton)
	{
		StartButton->SetVisibility(IsHost() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StartButton->SetIsEnabled(IsHost() && Num > 0 && ReadyCount == Num);
	}

	// Start's navigability may have just changed; re-validate the focused index.
	ApplyHighlight();
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
