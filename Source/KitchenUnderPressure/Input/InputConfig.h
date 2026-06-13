// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Bundles a gameplay input context (one or more IMCs at a shared priority) with the input
 * actions the pawn binds. Referenced once by the player controller so the input wiring lives
 * in a single asset instead of loose UInputAction pointers spread across the pawn and controller.
 */
UCLASS(BlueprintType)
class KITCHENUNDERPRESSURE_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Mapping contexts added to the Enhanced Input subsystem while gameplay input is active.
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	// Priority used when adding every context above (a higher priority wins over a lower one).
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction;

	// Pick up the aimed object, or drop the held one (the pawn toggles on this single action).
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> InteractAction;

	// Held to charge a throw, released to throw the carried object with variable force.
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrowAction;

	// Left mouse button: casts the left-hand spell (the hand mapping lives in the pawn).
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> CastLeftAction;

	// Right mouse button: casts the right-hand spell.
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> CastRightAction;
};
