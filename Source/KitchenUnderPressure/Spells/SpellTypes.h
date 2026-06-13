// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SpellTypes.generated.h"

/**
 * Which hand a spell is bound to. Each mouse button casts its matching hand (left mouse = left hand;
 * deliberately replaces the GDD 5.1 cross-wiring). Everything below the input layer is named by hand,
 * and the button-to-hand mapping lives in exactly one place: the pawn's DoCastLeft/DoCastRight.
 */
UENUM(BlueprintType)
enum class EHand : uint8
{
	Left,
	Right
};

/** Fired when a hand's equipped spell changes (server broadcasts directly, clients via OnRep). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandSpellChanged, EHand, Hand);

/** Borderlands-style rarity: how many parameters fall outside the norm and how many modifiers exist. */
UENUM(BlueprintType)
enum class ESpellRarity : uint8
{
	Common,
	Rare,
	Epic,
	Legendary
};

/**
 * A procedurally generated spell, described by tags + scalars only (no asset pointers) so it
 * replicates cheaply by value and is resolved to its data assets locally through
 * USpellGenerationSettings. The element + form + modifiers are the three GDD layers.
 */
USTRUCT(BlueprintType)
struct FSpellDefinition
{
	GENERATED_BODY()

	/** Element tag (KUPTags::Element_*): drives damage type, colour and status effect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FGameplayTag Element;

	/** Form tag (KUPTags::Form_*): selects the delivery ability (projectile, nova, ...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FGameplayTag Form;

	/** Optional behaviour modifiers (more present at higher rarity). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FGameplayTagContainer Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	ESpellRarity Rarity = ESpellRarity::Common;

	/** Overall strength multiplier; drives both damage and visual scale ("strong param = strong visual"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	float Power = 1.f;

	bool IsValid() const { return Element.IsValid() && Form.IsValid(); }
};
