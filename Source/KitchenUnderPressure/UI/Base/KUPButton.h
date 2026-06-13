// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "KUPButton.generated.h"

class UCommonTextBlock;

/**
 * Project button base. One shared parent so the look (via the assigned UCommonButtonStyle) and the
 * press/hover feedback are defined once and reused everywhere. Unlike the standard Button, this is
 * not a single-child panel: its caption lives inside the button Blueprint as an optional bound
 * CommonTextBlock (name it "ButtonTextBlock") driven by the per-instance ButtonText property, so
 * each placed button sets its own label in the details panel. Mirrors Lyra's ULyraButtonBase.
 */
UCLASS(Abstract, Blueprintable)
class KITCHENUNDERPRESSURE_API UKUPButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	// Set the caption at runtime (also updates the bound text block).
	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonText(const FText& InText);

protected:
	virtual void NativePreConstruct() override;

	// Caption shown on the button; set per placed instance. Pushed into ButtonTextBlock if bound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	FText ButtonText;

	// Optional label inside the button Blueprint. Add a CommonTextBlock named exactly "ButtonTextBlock".
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ButtonTextBlock;
};
