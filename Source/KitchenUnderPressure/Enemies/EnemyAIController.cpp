// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "KUPGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemyAIController::AEnemyAIController()
{
	// AI logic only runs on the server, where the controller possesses the pawn.
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BehaviorTimer, this, &AEnemyAIController::UpdateBehavior, UpdateInterval, /*bLoop=*/true);
	}
}

void AEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(BehaviorTimer);
	Super::OnUnPossess();
}

void AEnemyAIController::UpdateBehavior()
{
	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	APawn* Target = FindNearestPlayer();
	if (!Enemy || !Target)
	{
		return;
	}

	MoveToActor(Target, AcceptanceRadius);
	Enemy->TryMeleeAttack(Target);
}

APawn* AEnemyAIController::FindNearestPlayer() const
{
	UWorld* World = GetWorld();
	const APawn* SelfPawn = GetPawn();
	if (!World || !SelfPawn)
	{
		return nullptr;
	}

	APawn* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			continue;
		}

		// Dead players are not targets: chase the living instead of camping a body.
		const UAbilitySystemComponent* PlayerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn);
		if (PlayerASC && PlayerASC->HasMatchingGameplayTag(KUPTags::State_Dead))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), SelfPawn->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = PlayerPawn;
		}
	}
	return Best;
}
