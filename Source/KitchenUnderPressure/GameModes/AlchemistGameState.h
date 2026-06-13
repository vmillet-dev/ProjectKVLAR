// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AlchemistGameState.generated.h"

UENUM(BlueprintType)
enum class ERunState : uint8
{
	InProgress,
	Cleared, // every room cleared (win)
	Wiped    // all players dead (loss)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRunProgressChanged);

/**
 * Replicated, HUD-facing state of the current run: which room the party is in, how many rooms there
 * are, and whether the run is ongoing/won/lost. The game mode (server) writes these; the HUD reads
 * them and binds to OnRunProgressChanged.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AAlchemistGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RunProgress, Category = "Run")
	int32 CurrentRoomIndex = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RunProgress, Category = "Run")
	int32 TotalRooms = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RunProgress, Category = "Run")
	ERunState RunState = ERunState::InProgress;

	/** GDD 8.2 ("chaos" mode): when true, spells also damage other players. Written once by the game
	 *  mode at startup; every damage application reads it through UKUPCombatStatics::CanDamage. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Run")
	bool bFriendlyFire = false;

	/** Fired on clients whenever any run-progress value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRunProgressChanged OnRunProgressChanged;

protected:
	UFUNCTION()
	void OnRep_RunProgress();
};
