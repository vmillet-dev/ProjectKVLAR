// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlchemistGameState.h"
#include "Net/UnrealNetwork.h"

void AAlchemistGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlchemistGameState, CurrentRoomIndex);
	DOREPLIFETIME(AAlchemistGameState, TotalRooms);
	DOREPLIFETIME(AAlchemistGameState, RunState);
	DOREPLIFETIME(AAlchemistGameState, bFriendlyFire);
}

void AAlchemistGameState::OnRep_RunProgress()
{
	OnRunProgressChanged.Broadcast();
}
