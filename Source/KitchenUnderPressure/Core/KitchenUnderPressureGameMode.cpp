// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenUnderPressureGameMode.h"

AKitchenUnderPressureGameMode::AKitchenUnderPressureGameMode()
{
	// Match the lobby so a later ServerTravel from gameplay keeps every client connected
	// instead of forcing a full reconnect.
	bUseSeamlessTravel = true;
}
