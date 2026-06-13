// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Spells/SpellTypes.h"
#include "AlchemistPlayerState.generated.h"

class UAlchemistAbilitySystemComponent;
class UAlchemistAttributeSet;

/**
 * Holds the player's AbilitySystemComponent and AttributeSet. The ASC lives on the PlayerState (not
 * the pawn) so attributes, cooldowns and abilities survive pawn death/respawn and seamless travel
 * between dungeon maps. Replication mode is Mixed: GameplayEffects go only to the owning client,
 * tags/cues to everyone.
 *
 * The two-hand spell loadout also lives here (replicated) for the same reason: spells are the run's
 * loot, so they must survive the pawn. The pawn's USpellCasterComponent is only the executor that
 * grants/activates the matching abilities for whatever is stored here.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AAlchemistPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAlchemistPlayerState();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	UAlchemistAttributeSet* GetAlchemistAttributeSet() const { return AttributeSet; }

	const FSpellDefinition& GetSpell(EHand Hand) const { return Hand == EHand::Left ? LeftSpell : RightSpell; }

	/** Server: store a hand's spell (replicates to everyone; broadcasts OnHandSpellChanged locally). */
	void SetSpell(EHand Hand, const FSpellDefinition& Spell);

	/** True once the starting spells were rolled, so a respawned pawn re-equips instead of re-rolling. */
	bool HasRolledStartingSpells() const { return bStartingSpellsRolled; }
	void MarkStartingSpellsRolled() { bStartingSpellsRolled = true; }

	/** Fired on every machine when a hand's spell changes (UI / hand auras bind here). */
	UPROPERTY(BlueprintAssignable, Category = "Spells")
	FOnHandSpellChanged OnHandSpellChanged;

	// Granted ability bookkeeping, server-only. Stored here (not on the pawn component) so a respawned
	// pawn can clear the previous pawn's grants from the persistent ASC before re-granting.
	FGameplayAbilitySpecHandle LeftSpellHandle;
	FGameplayAbilitySpecHandle RightSpellHandle;
	FGameplayAbilitySpecHandle ComboHandle;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	TObjectPtr<UAlchemistAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAlchemistAttributeSet> AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_LeftSpell)
	FSpellDefinition LeftSpell;

	UPROPERTY(ReplicatedUsing = OnRep_RightSpell)
	FSpellDefinition RightSpell;

private:
	UFUNCTION()
	void OnRep_LeftSpell();

	UFUNCTION()
	void OnRep_RightSpell();

	bool bStartingSpellsRolled = false;
};
