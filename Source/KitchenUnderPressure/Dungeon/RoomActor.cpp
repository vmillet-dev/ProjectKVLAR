// Copyright Epic Games, Inc. All Rights Reserved.

#include "RoomActor.h"
#include "Enemies/EnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ARoomActor::ARoomActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(300.f, 300.f, 150.f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(SceneRoot);
}

void ARoomActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARoomActor, bDoorOpen);
}

void ARoomActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyDoorState();

	if (HasAuthority() && TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARoomActor::OnTriggerBeginOverlap);
	}
}

void ARoomActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || bActivated)
	{
		return;
	}

	// Only a player entering starts the fight.
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		SpawnWave();
	}
}

void ARoomActor::SpawnWave()
{
	if (!HasAuthority() || bActivated)
	{
		return;
	}
	bActivated = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// GDD 8.1: the wave scales in quantity and resistance with the number of players.
	int32 PlayerCount = 1;
	if (const AGameStateBase* GS = World->GetGameState())
	{
		PlayerCount = FMath::Max(1, GS->PlayerArray.Num());
	}
	const int32 ExtraPlayers = PlayerCount - 1;
	const int32 TotalCount = (Wave.Num() > 0)
		? FMath::CeilToInt(Wave.Num() * (1.f + WaveCountScalePerPlayer * ExtraPlayers))
		: 0;
	const float HealthScale = 1.f + HealthScalePerPlayer * ExtraPlayers;

	AliveCount = 0;
	for (int32 Index = 0; Index < TotalCount; ++Index)
	{
		TSubclassOf<AEnemyCharacter> EnemyClass = Wave[Index % Wave.Num()];
		if (!EnemyClass)
		{
			continue;
		}

		FVector SpawnLocation;
		const FRotator SpawnRotation = GetActorRotation();
		if (SpawnPoints.Num() > 0 && SpawnPoints[Index % SpawnPoints.Num()])
		{
			SpawnLocation = SpawnPoints[Index % SpawnPoints.Num()]->GetComponentLocation();
		}
		else
		{
			// Fan the wave out in a ring around the room if no explicit spawn points were placed.
			const float Angle = (360.f / FMath::Max(1, TotalCount)) * Index;
			SpawnLocation = GetActorLocation() + FRotator(0.f, Angle, 0.f).Vector() * 300.f;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams))
		{
			// BeginPlay (and the init attributes effect) ran inside SpawnActor, so scaling is safe here.
			Enemy->ApplyHealthScale(HealthScale);
			Enemy->OnEnemyDied.AddUObject(this, &ARoomActor::HandleEnemyDied);
			++AliveCount;
		}
	}

	if (AliveCount == 0)
	{
		// An empty wave is already cleared.
		bDoorOpen = true;
		ApplyDoorState();
		OnRoomCleared.Broadcast(this);
	}
}

void ARoomActor::HandleEnemyDied(AEnemyCharacter* Enemy)
{
	--AliveCount;
	if (AliveCount <= 0)
	{
		AliveCount = 0;
		bDoorOpen = true;
		ApplyDoorState();
		OnRoomCleared.Broadcast(this);
	}
}

void ARoomActor::OnRep_DoorOpen()
{
	ApplyDoorState();
}

void ARoomActor::ApplyDoorState()
{
	if (Door)
	{
		Door->SetVisibility(!bDoorOpen);
		Door->SetCollisionEnabled(bDoorOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	}
}
