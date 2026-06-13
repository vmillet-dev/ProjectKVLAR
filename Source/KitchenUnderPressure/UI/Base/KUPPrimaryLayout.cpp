// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPPrimaryLayout.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UCommonActivatableWidget* UKUPPrimaryLayout::PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!MenuStack || !WidgetClass)
	{
		return nullptr;
	}
	return MenuStack->AddWidget<UCommonActivatableWidget>(WidgetClass);
}

void UKUPPrimaryLayout::ClearMenuStack()
{
	if (MenuStack)
	{
		MenuStack->ClearWidgets();
	}
}
