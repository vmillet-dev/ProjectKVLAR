// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ComboBurst.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * The runtime visual for a two-hand combo: a coloured sphere that expands to the reaction radius and
 * fades out. Colour and radius come from the two combined spells and are replicated so every player
 * sees the same burst. Uses engine primitive content so it works without any authored asset (a
 * Blueprint child can replace it with nicer FX later).
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AComboBurst : public AActor
{
	GENERATED_BODY()

public:
	AComboBurst();

	/** Server: set the blended colour and the radius the sphere expands to. */
	void InitBurst(const FLinearColor& InColor, float InRadius);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "Combo")
	TObjectPtr<UStaticMeshComponent> MeshComp;

private:
	UFUNCTION()
	void OnRep_Visual();

	void ApplyVisual();

	UPROPERTY(ReplicatedUsing = OnRep_Visual)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(ReplicatedUsing = OnRep_Visual)
	float TargetRadius = 200.f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	float Age = 0.f;
	float Lifetime = 0.45f;
};
