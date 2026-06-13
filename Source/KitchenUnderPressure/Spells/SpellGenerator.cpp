// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellGenerator.h"
#include "SpellGenerationSettings.h"
#include "ElementDefinition.h"
#include "FormDefinition.h"
#include "ModifierDefinition.h"

ESpellRarity USpellGenerator::RollRarity(FRandomStream& Rng)
{
	const USpellGenerationSettings* Settings = GetDefault<USpellGenerationSettings>();
	if (!Settings)
	{
		return ESpellRarity::Common;
	}

	const float Total = Settings->CommonWeight + Settings->RareWeight + Settings->EpicWeight + Settings->LegendaryWeight;
	if (Total <= 0.f)
	{
		return ESpellRarity::Common;
	}

	// Walk the cumulative weights from common (most likely) to legendary.
	float Roll = Rng.FRandRange(0.f, Total);
	if ((Roll -= Settings->CommonWeight) < 0.f) { return ESpellRarity::Common; }
	if ((Roll -= Settings->RareWeight) < 0.f)   { return ESpellRarity::Rare; }
	if ((Roll -= Settings->EpicWeight) < 0.f)   { return ESpellRarity::Epic; }
	return ESpellRarity::Legendary;
}

FSpellDefinition USpellGenerator::GenerateRandomSpell(FRandomStream& Rng)
{
	FSpellDefinition Out;

	USpellGenerationSettings* Settings = GetMutableDefault<USpellGenerationSettings>();
	if (!Settings)
	{
		return Out;
	}

	const TArray<TObjectPtr<UElementDefinition>>& Elements = Settings->GetLoadedElements();
	const TArray<TObjectPtr<UFormDefinition>>& Forms = Settings->GetLoadedForms();
	const TArray<TObjectPtr<UModifierDefinition>>& Modifiers = Settings->GetLoadedModifiers();
	if (Elements.Num() == 0 || Forms.Num() == 0)
	{
		// Leaves Out invalid; callers guard on IsValid() and skip equipping.
		return Out;
	}

	const ESpellRarity Rarity = RollRarity(Rng);
	Out.Rarity = Rarity;
	Out.Power = Settings->GetPowerForRarity(Rarity) * Rng.FRandRange(0.9f, 1.1f);

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
