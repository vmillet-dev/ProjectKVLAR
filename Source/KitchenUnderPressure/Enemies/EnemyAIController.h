// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * Minimal server-only chase AI: on a fixed interval it finds the nearest player pawn, paths toward it
 * (MoveToActor, needs a NavMeshBoundsVolume in the level) and asks its enemy to melee when close.
 * Deliberately tiny for the first slice; StateTree can replace it later without touching combat code.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float UpdateInterval = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AcceptanceRadius = 120.f;

private:
	void UpdateBehavior();
	APawn* FindNearestPlayer() const;

	FTimerHandle BehaviorTimer;
};
