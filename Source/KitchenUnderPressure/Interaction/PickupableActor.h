// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupableActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/**
 *  Base class for a physics object that can be picked up, carried and thrown.
 *  The server is authoritative. While carried, the object attaches to AttachTarget (a carrier's
 *  hold point) and stops simulating. AttachTarget is replicated so every client applies the same
 *  "stop physics, then attach" in a single step. That ordering matters: attaching a body that is
 *  still simulating only links it in name, and it never follows the carrier — the classic
 *  held-object-stays-put-on-other-clients bug.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API APickupableActor : public AActor
{
	GENERATED_BODY()

public:
	APickupableActor();

	/** Server-only: attach onto the carrier's hold point and stop simulating. */
	void OnPickedUp(USceneComponent* AttachTo);

	/** Server-only: detach, resume physics and apply a launch velocity (zero = gentle drop). */
	void OnReleased(const FVector& LaunchVelocity);

	/** True while carried. */
	bool IsHeld() const { return AttachTarget != nullptr; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Static mesh (the cube) and physics root. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComp;

private:
	/** Carrier hold point we attach to while held, or null when free. Single replicated source of
	 *  truth that drives physics, collision and attachment on the server and every client. */
	UPROPERTY(ReplicatedUsing = OnRep_AttachState)
	TObjectPtr<USceneComponent> AttachTarget;

	UFUNCTION()
	void OnRep_AttachState();

	/** Applies physics, collision and attachment for the current AttachTarget (server and clients). */
	void ApplyAttachState();
};
