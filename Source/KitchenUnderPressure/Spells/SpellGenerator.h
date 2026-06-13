// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Spells/SpellTypes.h"
#include "SpellGenerator.generated.h"

/**
 * Server-side spell roller. Reads USpellGenerationSettings, rolls a rarity from the configured
 * weights, then fills the element / form / modifier layers from the pools. Stateless given the
 * settings; pass an FRandomStream so a run can be seeded and reproduced.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API USpellGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Roll a complete random spell. Returns an invalid spell if the pools are empty. */
	static FSpellDefinition GenerateRandomSpell(FRandomStream& Rng);

	/** Roll only the rarity tier from the configured weights. */
	static ESpellRarity RollRarity(FRandomStream& Rng);
};
