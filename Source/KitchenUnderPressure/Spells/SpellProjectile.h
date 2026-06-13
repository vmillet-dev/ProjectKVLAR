// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "SpellProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UMaterialInstanceDynamic;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * A replicated spell projectile. Spawned and launched on the server by a projectile-form ability; on
 * a blocking hit (server) it applies the damage GE and the element's status GE to the victim's ability
 * system, then destroys itself. Element colour and visual scale are replicated so every client renders
 * the same projectile (stronger spells are visibly bigger and brighter).
 */
UCLASS()
class KITCHENUNDERPRESSURE_API ASpellProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpellProjectile();

	/** Server: set velocity, damage/status/element payload, colour and visual scale right after spawning. */
	void InitProjectile(const FVector& Velocity, float InDamage, TSubclassOf<UGameplayEffect> InDamageEffect, TSubclassOf<UGameplayEffect> InStatusEffect, UAbilitySystemComponent* InInstigatorASC, const FLinearColor& InColor, float InVisualScale, const FGameplayTag& InElement);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

private:
	UFUNCTION()
	void OnRep_Visual();

	/** Applies the element colour and the power-driven scale (runs on server and clients). */
	void ApplyVisual();

	/** Replicated so every client renders the projectile in its element colour. */
	UPROPERTY(ReplicatedUsing = OnRep_Visual)
	FLinearColor Color = FLinearColor::White;

	/** Replicated visual/hit scale; grows with the spell's power. */
	UPROPERTY(ReplicatedUsing = OnRep_Visual)
	float VisualScale = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// Server-only damage payload (never replicated; the server applies the effects).
	float Damage = 0.f;
	TSubclassOf<UGameplayEffect> DamageEffect;
	TSubclassOf<UGameplayEffect> StatusEffect;
	TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC;

	/** Element carried by this projectile; reported to struck enemies for cross-player combos. */
	FGameplayTag Element;
};
