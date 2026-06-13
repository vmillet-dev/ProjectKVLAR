// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "Templates/SubclassOf.h"
#include "PickupComponent.generated.h"

class APickupableActor;
class UThrowChargeWidget;

/**
 *  Lets the owning character pick up, drop and throw an APickupableActor.
 *  Every state change is server-authoritative (Server RPCs); the carried actor is replicated so
 *  all clients see who holds what. Throw force scales with how long the throw button is held.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KITCHENUNDERPRESSURE_API UPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPickupComponent();

	/** Pick up the aimed actor, or drop the held one (same input toggles both). */
	void Interact();

	/** Begin measuring the throw charge (local; only while holding). */
	void StartThrowCharge();

	/** Release the throw: maps the held duration to a launch speed and asks the server to throw. */
	void ReleaseThrow();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Ticks only while charging a throw, to push the live charge value to the HUD bar.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Aim trace length when picking up (cm). */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float ReachDistance = 250.f;

	/** Sphere radius added to the aim trace for some tolerance (cm). */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float TraceRadius = 18.f;

	/** Launch speed of a zero-charge throw (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Pickup|Throw")
	float MinThrowSpeed = 300.f;

	/** Launch speed of a fully charged throw (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Pickup|Throw")
	float MaxThrowSpeed = 1500.f;

	/** Hold duration (seconds) that reaches MaxThrowSpeed. */
	UPROPERTY(EditAnywhere, Category = "Pickup|Throw")
	float MaxChargeTime = 1.5f;

	/** HUD widget (a UThrowChargeWidget Blueprint) shown while charging a throw. */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup|UI")
	TSubclassOf<UThrowChargeWidget> ChargeWidgetClass;

private:
	UFUNCTION(Server, Reliable)
	void ServerTryPickup();

	UFUNCTION(Server, Reliable)
	void ServerDrop();

	UFUNCTION(Server, Reliable)
	void ServerThrow(FVector_NetQuantizeNormal Direction, float ChargeRatio);

	/** Server: sphere-trace from the owner's eyes for a free pickupable actor. */
	APickupableActor* TraceForPickup() const;

	/** Server: attach Target and remember it. */
	void Pickup(APickupableActor* Target);

	/** Server: release the held actor with the given launch velocity (zero = gentle drop). */
	void Release(const FVector& LaunchVelocity);

	/** Eyes location + forward direction of the owning pawn. */
	void GetAimPoint(FVector& OutLocation, FVector& OutDirection) const;

	/** Current throw charge in [0,1] (0 when not charging). */
	float GetChargeRatio() const;

	/** Lazily creates the HUD bar on the local owning client. */
	void EnsureChargeWidget();

	/** The actor currently carried (replicated so every client sees it). */
	UPROPERTY(Replicated)
	TObjectPtr<APickupableActor> HeldActor;

	/** HUD bar instance, created on the local owning client only. */
	UPROPERTY()
	TObjectPtr<UThrowChargeWidget> ChargeWidget;

	/** Local-only throw charge tracking on the owning client. */
	float ChargeStartTime = 0.f;
	bool bCharging = false;
};
