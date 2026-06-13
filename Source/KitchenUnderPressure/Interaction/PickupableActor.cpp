// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/PickupableActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

APickupableActor::APickupableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// The server simulates the physics; the resulting movement is replicated to every client.
	bReplicates = true;
	SetReplicateMovement(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetSimulatePhysics(true);

	// PhysicsActor already simulates; force-block Visibility so the carrier's aim trace hits it
	// regardless of the engine's default profile responses.
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default to the engine cube so a derived Blueprint is usable straight away.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(CubeMesh.Object);
	}
}

void APickupableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickupableActor, AttachTarget);
}

void APickupableActor::OnPickedUp(USceneComponent* AttachTo)
{
	if (!AttachTo)
	{
		return;
	}

	AttachTarget = AttachTo;
	ApplyAttachState();
}

void APickupableActor::OnReleased(const FVector& LaunchVelocity)
{
	AttachTarget = nullptr;
	ApplyAttachState();

	// bVelChange = true: LaunchVelocity is a velocity change (cm/s), so the throw feel is
	// independent of the cube's mass and easy to tune.
	if (!LaunchVelocity.IsNearlyZero())
	{
		MeshComp->AddImpulse(LaunchVelocity, NAME_None, /*bVelChange=*/true);
	}
}

void APickupableActor::OnRep_AttachState()
{
	ApplyAttachState();
}

void APickupableActor::ApplyAttachState()
{
	if (AttachTarget)
	{
		// Order matters: stop simulating BEFORE attaching. Attaching a still-simulating body only
		// links it in name, so it never follows the carrier on clients.
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);
	}
}
