// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPGameUserSettings.h"
#include "Engine/Engine.h"

UKUPGameUserSettings* UKUPGameUserSettings::Get()
{
	return GEngine ? Cast<UKUPGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}
