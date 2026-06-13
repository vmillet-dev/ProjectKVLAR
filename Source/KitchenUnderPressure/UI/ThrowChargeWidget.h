// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrowChargeWidget.generated.h"

class UProgressBar;

/**
 * HUD bar shown while a throw is being charged. The owning UPickupComponent pushes the current
 * charge (0..1) each frame via SetChargePercent and toggles visibility with Show/Hide. Purely
 * local and cosmetic — the real throw force is computed and clamped on the server.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UThrowChargeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the fill amount (clamped to 0..1). */
	void SetChargePercent(float Percent);

	/** Make the bar visible (non-interactive). */
	void Show();

	/** Hide the bar. */
	void Hide();

protected:
	virtual void NativeConstruct() override;

	// Fill bar driven by the throw charge. Name it "ChargeBar" in the Blueprint.
	UPROPERTY(meta = (BindWidget)) UProgressBar* ChargeBar;
};
