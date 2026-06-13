// Fill out your copyright notice in the Description page of Project Settings.

#include "ThrowChargeWidget.h"
#include "Components/ProgressBar.h"

void UThrowChargeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden; the pickup component shows the bar when a throw starts charging.
	Hide();
}

void UThrowChargeWidget::SetChargePercent(float Percent)
{
	if (ChargeBar)
	{
		ChargeBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void UThrowChargeWidget::Show()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UThrowChargeWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
