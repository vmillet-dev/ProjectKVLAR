// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Spells/SpellTypes.h"
#include "SpellGenerator.generated.h"

class USpellConfig;
class USpellRegistrySubsystem;

/**
 * Server-side spell roller. Rolls a rarity from the config's weights, then fills the element / form /
 * modifier layers from the registry's loaded pools. Stateless given the registry; pass an
 * FRandomStream so a run can be seeded and reproduced.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API USpellGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Roll a complete random spell. Returns an invalid spell if the registry or pools are empty.
	static FSpellDefinition GenerateRandomSpell(USpellRegistrySubsystem* Registry, FRandomStream& Rng);

	// Roll only the rarity tier from the config's weights.
	static ESpellRarity RollRarity(const USpellConfig* Config, FRandomStream& Rng);
};
