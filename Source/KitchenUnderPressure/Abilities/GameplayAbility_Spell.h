// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Spells/SpellTypes.h"
#include "GameplayAbility_Spell.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * Base class for every spell form (projectile, nova, ...). Activated authoritatively on the server
 * (the caster component routes a Server RPC), so the net execution policy is ServerOnly and there is
 * no client-side prediction.
 *
 * Forms read their payload through ResolvePayload(), which pulls the equipped FSpellDefinition from
 * the caster (the spec SourceObject), resolves its element/form/modifiers to data assets, and computes
 * damage / colour / status. When no spell is equipped it falls back to this object's own defaults, so
 * a purely Blueprint-configured ability still works.
 */
UCLASS(Abstract)
class KITCHENUNDERPRESSURE_API UGameplayAbility_Spell : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Spell();

	//~ Server-side fire-rate guard (see CooldownSeconds).
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

protected:
	/** Muzzle position and aim direction. The muzzle sits beside the avatar's eyes on this ability's
	 *  hand side (left spells leave from the left, right from the right) and the direction converges
	 *  on the crosshair (aim trace from the eyes), so both hands still hit where the player aims.
	 *  Falls back to the centred eye position when the ability has no hand (e.g. the combo). */
	void GetMuzzle(FVector& OutLocation, FVector& OutDirection) const;

	/** Reads the spell equipped to this ability's hand from the caster (the spec SourceObject). */
	bool TryGetEquippedSpell(FSpellDefinition& OutSpell) const;

	/** Which hand this ability instance is granted to (via the caster and the spec handle). */
	bool TryGetHand(EHand& OutHand) const;

	/** Fire rate actually enforced: the equipped spell's form CooldownSeconds, or this ability's
	 *  CooldownSeconds when no spell is equipped. */
	float GetEffectiveCooldownSeconds() const;

	/** Element tag of the equipped spell (invalid tag when nothing is equipped). */
	FGameplayTag GetEquippedElement() const;

	/** Resolves damage / colour / status / visual scale from the equipped spell, or this ability's
	 *  defaults. Modifiers (layer 3) scale the damage; visual scale grows with the spell's power so
	 *  stronger spells look bigger. */
	void ResolvePayload(float& OutDamage, FLinearColor& OutColor, TSubclassOf<UGameplayEffect>& OutStatusEffect, float& OutVisualScale) const;

	/** Applies the damage GE (SetByCaller Data.Damage) and the optional status GE to a target ASC. */
	void ApplyDamageAndStatus(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InDamage, TSubclassOf<UGameplayEffect> StatusEffect) const;

	/** The shared damage GameplayEffect (from the Alchemist Spells settings; SetByCaller Data.Damage). */
	TSubclassOf<UGameplayEffect> GetDamageEffectClass() const;

	/** Fallback damage used only when no spell is equipped. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float Damage = 25.f;

	/** Fallback colour used only when no spell is equipped. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	FLinearColor ElementColor = FLinearColor(1.f, 0.35f, 0.05f);

	/** Server-side minimum time between two commits of this spell (per hand, since the ability is
	 *  instanced per spec). Only a fallback: the equipped spell's form CooldownSeconds takes priority
	 *  (see GetEffectiveCooldownSeconds). 0 disables the guard. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float CooldownSeconds = 0.5f;

	/** Sideways distance from the eyes to the casting hand's muzzle, in cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float MuzzleLateralOffset = 30.f;

	/** Vertical muzzle offset from the eyes, in cm (negative = below eye height, like a held hand). */
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float MuzzleVerticalOffset = -15.f;

private:
	/** World time of the last successful commit (server-only; the ability never runs on clients). */
	float LastCommitTime = -1000.f;
};
