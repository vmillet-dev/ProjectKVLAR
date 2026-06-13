// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Spells/SpellTypes.h"
#include "SpellCasterComponent.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class UGameplayEffect;
class AAlchemistPlayerState;

/** What a hand last cast; used to detect two-hand combos. Server-side, not replicated. */
struct FHandCastRecord
{
	FGameplayTag Element;
	FGameplayTagContainer Modifiers;
	float Time = -1000.f;
};

/** A pre-computed combo handed to the reaction ability on activation. Server-side, not replicated. */
struct FPendingCombo
{
	FVector Location = FVector::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float Damage = 0.f;
	float Radius = 300.f;
	TSubclassOf<UGameplayEffect> StatusEffect = nullptr;
};

/**
 * Bridges "a hand has a spell" to "a granted GameplayAbility". Lives on the player pawn; the spell
 * loadout itself (the run's loot) and the granted spec handles are stored on AAlchemistPlayerState so
 * they survive pawn death/respawn and seamless travel — this component only executes against them.
 *
 * Equipping resolves the spell's Form to its ability class and (re)grants it server-side; the granted
 * ability reads the definition back via the spec's SourceObject (this component) and the per-hand spec
 * handle. When both hands successfully cast within the combo window, an alchemical reaction is
 * triggered. All granting/activation is server-authoritative.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KITCHENUNDERPRESSURE_API USpellCasterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpellCasterComponent();

	/** Server: equip the loadout stored on the PlayerState (rolling it first if this is a fresh run)
	 *  and grant the combo ability. Called by the character once the ability system is bound to the
	 *  pawn; on a respawned pawn this re-grants abilities for the persisted spells instead of re-rolling. */
	void ServerInitializeAbilities();

	/** Server: equip a specific spell in a hand (stores it on the PlayerState, re-grants the form ability). */
	void EquipSpell(EHand Hand, const FSpellDefinition& Spell);

	/** Owning client: ask the server to cast the spell bound to the given hand. */
	void TryCast(EHand Hand);

	const FSpellDefinition& GetSpell(EHand Hand) const;

	/** Resolve the spell for a granted ability spec handle (used by the form ability on activation). */
	bool GetSpellForHandle(FGameplayAbilitySpecHandle Handle, FSpellDefinition& OutSpell) const;

	/** Resolve which hand a granted ability spec handle belongs to (drives the muzzle side). */
	bool GetHandForHandle(FGameplayAbilitySpecHandle Handle, EHand& OutHand) const;

	/** The combo computed at the last reaction trigger (read by the reaction ability on activation). */
	const FPendingCombo& GetPendingCombo() const { return PendingCombo; }

protected:
	/** Used only if a spell's form can't be resolved to an ability (optional safety net). */
	UPROPERTY(EditDefaultsOnly, Category = "Spells")
	TSubclassOf<UGameplayAbility> FallbackSpellAbility;

	/** Ability run when a two-hand combo fires (assign BP_GA_ComboReaction). */
	UPROPERTY(EditDefaultsOnly, Category = "Spells|Combo")
	TSubclassOf<UGameplayAbility> ComboAbility;

	/** Radius of the combo reaction burst in cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Spells|Combo")
	float ComboRadius = 350.f;

private:
	UFUNCTION(Server, Reliable)
	void ServerActivateSpell(EHand Hand);

	UAbilitySystemComponent* GetASC() const;
	AAlchemistPlayerState* GetAlchemistPlayerState() const;

	/** Record this hand's cast and, if the other hand cast within the window, fire the combo. */
	void RecordCastAndTryCombo(EHand Hand);

	/** Classify the two recorded casts, compute the reaction, and activate the combo ability. */
	void TriggerCombo();

	/** Aim point where the combo burst is centred (line trace from the owner's eyes). */
	FVector ComputeComboLocation() const;

	FHandCastRecord LastLeftCast;
	FHandCastRecord LastRightCast;
	FPendingCombo PendingCombo;

	/** Per-pawn guard: abilities were already granted for this pawn's lifetime. */
	bool bAbilitiesGranted = false;
};
