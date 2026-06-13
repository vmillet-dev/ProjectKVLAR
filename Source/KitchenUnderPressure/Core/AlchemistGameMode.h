// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KitchenUnderPressureGameMode.h"
#include "AlchemistGameMode.generated.h"

class ARoomActor;

/**
 * Gameplay game mode for an Alchemist run. Selects the project GameState/PlayerState classes, gathers
 * the hand-placed rooms at start, and advances the run as rooms are cleared (win on the last room,
 * wipe when every player is down). Reparent BP_FirstPersonGameMode to this so Lvl_FirstPerson keeps
 * using its Blueprint pawn/controller while gaining the run + GAS wiring.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AAlchemistGameMode : public AKitchenUnderPressureGameMode
{
	GENERATED_BODY()

public:
	AAlchemistGameMode();

	/** Server: a player pawn died; if everyone is down the run is wiped. */
	void NotifyPlayerDied(AController* DeadPlayer);

protected:
	virtual void BeginPlay() override;
	virtual void InitGameState() override;

	/** GDD 8.2: optional friendly fire ("chaos" mode). Copied to the game state so every machine and
	 *  every damage site reads the same value. */
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	bool bFriendlyFire = false;

private:
	void GatherRooms();
	void HandleRoomCleared(ARoomActor* Room);

	UPROPERTY(Transient)
	TArray<TObjectPtr<ARoomActor>> Rooms;
};
