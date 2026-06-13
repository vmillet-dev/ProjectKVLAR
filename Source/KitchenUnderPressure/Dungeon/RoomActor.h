// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "RoomActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;
class AEnemyCharacter;

/** Server-side broadcast when a room is cleared (the game mode binds to advance the run). */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomCleared, class ARoomActor*);

/**
 * A hand-placed dungeon room. When a player first enters the trigger, it spawns its enemy wave and
 * tracks deaths; once every enemy is dead it opens its door and reports the clear. Door state is
 * replicated so the opening is visible on all clients. Server-authoritative.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomActor();

	FOnRoomCleared OnRoomCleared;
	int32 GetRoomIndex() const { return RoomIndex; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Room")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Room")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, Category = "Room")
	TObjectPtr<UStaticMeshComponent> Door;

	/** Optional explicit spawn points (child scene components). If empty, enemies spawn in a ring. */
	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<TObjectPtr<USceneComponent>> SpawnPoints;

	/** Order of this room within the run (lower = earlier). */
	UPROPERTY(EditAnywhere, Category = "Room")
	int32 RoomIndex = 0;

	/** Enemy classes spawned when the room activates (one actor per entry, for a solo run; the wave
	 *  cycles through these entries when player-count scaling adds extras). */
	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<TSubclassOf<AEnemyCharacter>> Wave;

	/** GDD 8.1: extra enemies per additional player, as a fraction of the base wave
	 *  (0.5 = +50% of the wave per extra player; 2 players on a 4-enemy wave spawn 6). */
	UPROPERTY(EditAnywhere, Category = "Room|Scaling", meta = (ClampMin = "0.0"))
	float WaveCountScalePerPlayer = 0.5f;

	/** GDD 8.1: extra enemy max health per additional player (0.3 = +30% per extra player). */
	UPROPERTY(EditAnywhere, Category = "Room|Scaling", meta = (ClampMin = "0.0"))
	float HealthScalePerPlayer = 0.3f;

private:
	UFUNCTION()
	void OnRep_DoorOpen();

	void ApplyDoorState();
	void SpawnWave();
	void HandleEnemyDied(AEnemyCharacter* Enemy);

	UPROPERTY(ReplicatedUsing = OnRep_DoorOpen)
	bool bDoorOpen = false;

	bool bActivated = false;
	int32 AliveCount = 0;
};
