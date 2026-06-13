// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellConfig.h"

float USpellConfig::GetPowerForRarity(ESpellRarity Rarity) const
{
	switch (Rarity)
	{
	case ESpellRarity::Rare:      return RarePower;
	case ESpellRarity::Epic:      return EpicPower;
	case ESpellRarity::Legendary: return LegendaryPower;
	default:                      return CommonPower;
	}
}

bool USpellConfig::AreElementsOpposed(const FGameplayTag& A, const FGameplayTag& B) const
{
	for (const FElementOpposition& Pair : Oppositions)
	{
		if ((Pair.ElementA == A && Pair.ElementB == B) || (Pair.ElementA == B && Pair.ElementB == A))
		{
			return true;
		}
	}
	return false;
}
