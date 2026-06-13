// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AlchemistAbilitySystemComponent.generated.h"

/**
 * Project ability system component. Currently a thin subclass; it exists from the start so the
 * PlayerState/enemy assets and the spell-granting code never need reparenting when input-tag
 * activation and combo bookkeeping helpers are added later.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAlchemistAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
};
