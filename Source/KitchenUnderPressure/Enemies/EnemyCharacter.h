// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "EnemyCharacter.generated.h"

class UAlchemistAbilitySystemComponent;
class UAlchemistAttributeSet;
class UGameplayEffect;
class UAbilitySystemComponent;

/** Server-side broadcast when an enemy dies (the room binds to it to track its clear count). */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, class AEnemyCharacter*);

/** One elemental spell hit recently landed on this enemy. Server-side, not replicated. */
struct FElementalHitRecord
{
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	FGameplayTag Element;
	float Time = -1000.f;
};

/**
 * A simple melee enemy. Owns its own ability system (on the pawn, Minimal replication) and attribute
 * set; chased onto players by AEnemyAIController, it melees when in range. Also tracks the elemental
 * hits it receives so two different players landing opposed elements within the cross-player window
 * trigger a reaction (GDD 8.1). All combat logic is server-authoritative.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FOnEnemyDied OnEnemyDied;

	/** Server: melee the target if it is within range and the cooldown has elapsed. */
	void TryMeleeAttack(AActor* Target);

	/** Server: a spell of the given element hit this enemy. Records it, and if another player landed
	 *  the opposed element within the cross-player combo window, triggers the reaction (GDD 8.1). */
	void NotifyElementalHit(UAbilitySystemComponent* SourceASC, const FGameplayTag& Element);

	/** Server: scale max health (then heal to full) for the player-count difficulty (GDD 8.1).
	 *  Called by the room right after spawning, once the init attributes have been applied. */
	void ApplyHealthScale(float Multiplier);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	TObjectPtr<UAlchemistAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAlchemistAttributeSet> AttributeSet;

	/** Sets starting Health/MaxHealth/MoveSpeed on spawn (assign GE_EnemyInit). */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	/** Damage GE applied to a player on a melee hit; uses SetByCaller Data.Damage. Assign GE_Damage. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	TSubclassOf<UGameplayEffect> MeleeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MeleeDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MeleeRange = 160.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MeleeCooldown = 1.5f;

private:
	void HandleOutOfHealth(AActor* DeadAvatar);

	/** Apply the cross-player reaction burst centred on this enemy and spawn its visual. */
	void TriggerCrossPlayerCombo(const FElementalHitRecord& FirstHit, UAbilitySystemComponent* SecondASC, const FGameplayTag& SecondElement);

	/** Recent elemental hits within the cross-player combo window (pruned as new hits land). */
	TArray<FElementalHitRecord> RecentElementalHits;

	bool bDead = false;
	float LastMeleeTime = -100.f;
};
