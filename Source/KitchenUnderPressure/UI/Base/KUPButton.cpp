// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPButton.h"
#include "CommonTextBlock.h"

void UKUPButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Runs in the designer too, so the caption previews live when ButtonText is edited.
	if (ButtonTextBlock)
	{
		ButtonTextBlock->SetText(ButtonText);
	}
}

void UKUPButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;
	if (ButtonTextBlock)
	{
		ButtonTextBlock->SetText(InText);
	}
}
