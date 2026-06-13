// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellGenerator.h"
#include "SpellConfig.h"
#include "SpellRegistrySubsystem.h"
#include "ElementDefinition.h"
#include "FormDefinition.h"
#include "ModifierDefinition.h"

ESpellRarity USpellGenerator::RollRarity(const USpellConfig* Config, FRandomStream& Rng)
{
	if (!Config)
	{
		return ESpellRarity::Common;
	}

	const float Total = Config->CommonWeight + Config->RareWeight + Config->EpicWeight + Config->LegendaryWeight;
	if (Total <= 0.f)
	{
		return ESpellRarity::Common;
	}

	// Walk the cumulative weights from common (most likely) to legendary.
	float Roll = Rng.FRandRange(0.f, Total);
	if ((Roll -= Config->CommonWeight) < 0.f) { return ESpellRarity::Common; }
	if ((Roll -= Config->RareWeight) < 0.f)   { return ESpellRarity::Rare; }
	if ((Roll -= Config->EpicWeight) < 0.f)   { return ESpellRarity::Epic; }
	return ESpellRarity::Legendary;
}

FSpellDefinition USpellGenerator::GenerateRandomSpell(USpellRegistrySubsystem* Registry, FRandomStream& Rng)
{
	FSpellDefinition Out;

	if (!Registry)
	{
		return Out;
	}

	const USpellConfig* Config = Registry->GetConfig();
	if (!Config)
	{
		return Out;
	}

	const TArray<TObjectPtr<UElementDefinition>>& Elements = Registry->GetLoadedElements();
	const TArray<TObjectPtr<UFormDefinition>>& Forms = Registry->GetLoadedForms();
	const TArray<TObjectPtr<UModifierDefinition>>& Modifiers = Registry->GetLoadedModifiers();
	if (Elements.Num() == 0 || Forms.Num() == 0)
	{
		// Leaves Out invalid; callers guard on IsValid() and skip equipping.
		return Out;
	}

	const ESpellRarity Rarity = RollRarity(Config, Rng);
	Out.Rarity = Rarity;
	Out.Power = Config->GetPowerForRarity(Rarity) * Rng.FRandRange(0.9f, 1.1f);

	if (const UElementDefinition* Element = Elements[Rng.RandRange(0, Elements.Num() - 1)])
	{
		Out.Element = Element->ElementTag;
	}
	if (const UFormDefinition* Form = Forms[Rng.RandRange(0, Forms.Num() - 1)])
	{
		Out.Form = Form->FormTag;
	}

	// Modifier count rises with rarity (common 0, rare 1, epic 2, legendary 1 "unique").
	int32 ModifierCount = 0;
	switch (Rarity)
	{
	case ESpellRarity::Rare:      ModifierCount = 1; break;
	case ESpellRarity::Epic:      ModifierCount = 2; break;
	case ESpellRarity::Legendary: ModifierCount = 1; break;
	default:                      ModifierCount = 0; break;
	}

	for (int32 i = 0; i < ModifierCount && Modifiers.Num() > 0; ++i)
	{
		if (const UModifierDefinition* Modifier = Modifiers[Rng.RandRange(0, Modifiers.Num() - 1)])
		{
			if (Modifier->ModifierTag.IsValid())
			{
				Out.Modifiers.AddTag(Modifier->ModifierTag);
			}
		}
	}

	return Out;
}
