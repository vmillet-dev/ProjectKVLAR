// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPUISubsystem.h"
#include "KUPUISettings.h"
#include "Base/KUPPrimaryLayout.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

UKUPPrimaryLayout* UKUPUISubsystem::EnsureLayout()
{
	ULocalPlayer* LP = GetLocalPlayer();
	APlayerController* PC = LP ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
	if (!PC)
	{
		return nullptr;
	}

	// Reuse the layout only while it belongs to the current world; after a level travel the old
	// widget is gone, so rebuild it for the new world.
	if (Layout && Layout->GetWorld() == PC->GetWorld())
	{
		if (!Layout->IsInViewport())
		{
			Layout->AddToViewport();
		}
		return Layout;
	}

	const UKUPUISettings* Settings = GetDefault<UKUPUISettings>();
	TSubclassOf<UKUPPrimaryLayout> LayoutClass = Settings ? Settings->PrimaryLayout.LoadSynchronous() : nullptr;
	if (!LayoutClass)
	{
		return nullptr;
	}

	Layout = CreateWidget<UKUPPrimaryLayout>(PC, LayoutClass);
	if (Layout)
	{
		Layout->AddToViewport();
	}
	return Layout;
}

UCommonActivatableWidget* UKUPUISubsystem::PushScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass)
{
	UKUPPrimaryLayout* L = EnsureLayout();
	return L ? L->PushWidget(ScreenClass) : nullptr;
}

void UKUPUISubsystem::ClearScreens()
{
	if (Layout)
	{
		Layout->ClearMenuStack();
	}
}
