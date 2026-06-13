// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRowWidget.h"
#include "OnlineSessionSubsystem.h"
#include "Base/KUPButton.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UServerRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinRowButton)
	{
		JoinRowButton->OnClicked().AddUObject(this, &UServerRowWidget::OnJoinRowClicked);
	}
}

void UServerRowWidget::Setup(const FGameSessionInfo& Info, int32 Index)
{
	SearchResultIndex = Index;

	if (OwnerText) OwnerText->SetText(FText::FromString(Info.OwnerName));
	if (SlotsText) SlotsText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Info.OpenSlots, Info.MaxSlots)));
	if (PingText)  PingText->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), Info.PingMs)));
}

void UServerRowWidget::OnJoinRowClicked()
{
	if (SearchResultIndex < 0) return;

	// The subsystem lives on the GameInstance, so the row can join directly without
	// holding a reference back to the parent menu widget.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* Online = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			Online->JoinGame(SearchResultIndex);
		}
	}
}
