// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellGenerationSettings.h"
#include "ElementDefinition.h"
#include "FormDefinition.h"
#include "ModifierDefinition.h"

void USpellGenerationSettings::EnsureRegistry()
{
	if (bRegistryBuilt)
	{
		return;
	}
	bRegistryBuilt = true;

	for (const TSoftObjectPtr<UElementDefinition>& Soft : ElementPool)
	{
		if (UElementDefinition* Element = Soft.LoadSynchronous())
		{
			LoadedElements.Add(Element);
			if (Element->ElementTag.IsValid())
			{
				ElementByTag.Add(Element->ElementTag, Element);
			}
		}
	}

	for (const TSoftObjectPtr<UFormDefinition>& Soft : FormPool)
	{
		if (UFormDefinition* Form = Soft.LoadSynchronous())
		{
			LoadedForms.Add(Form);
			if (Form->FormTag.IsValid())
			{
				FormByTag.Add(Form->FormTag, Form);
			}
		}
	}

	for (const TSoftObjectPtr<UModifierDefinition>& Soft : ModifierPool)
	{
		if (UModifierDefinition* Modifier = Soft.LoadSynchronous())
		{
			LoadedModifiers.Add(Modifier);
			if (Modifier->ModifierTag.IsValid())
			{
				ModifierByTag.Add(Modifier->ModifierTag, Modifier);
			}
		}
	}
}

UElementDefinition* USpellGenerationSettings::FindElement(const FGameplayTag& Tag)
{
	EnsureRegistry();
	if (UElementDefinition** Found = ElementByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

UFormDefinition* USpellGenerationSettings::FindForm(const FGameplayTag& Tag)
{
	EnsureRegistry();
	if (UFormDefinition** Found = FormByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

UModifierDefinition* USpellGenerationSettings::FindModifier(const FGameplayTag& Tag)
{
	EnsureRegistry();
	if (UModifierDefinition** Found = ModifierByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

const TArray<TObjectPtr<UElementDefinition>>& USpellGenerationSettings::GetLoadedElements()
{
	EnsureRegistry();
	return LoadedElements;
}

const TArray<TObjectPtr<UFormDefinition>>& USpellGenerationSettings::GetLoadedForms()
{
	EnsureRegistry();
	return LoadedForms;
}

const TArray<TObjectPtr<UModifierDefinition>>& USpellGenerationSettings::GetLoadedModifiers()
{
	EnsureRegistry();
	return LoadedModifiers;
}

float USpellGenerationSettings::GetPowerForRarity(ESpellRarity Rarity) const
{
	switch (Rarity)
	{
	case ESpellRarity::Rare:      return RarePower;
	case ESpellRarity::Epic:      return EpicPower;
	case ESpellRarity::Legendary: return LegendaryPower;
	default:                      return CommonPower;
	}
}

float USpellGenerationSettings::GetWeightForRarity(ESpellRarity Rarity) const
{
	switch (Rarity)
	{
	case ESpellRarity::Rare:      return RareWeight;
	case ESpellRarity::Epic:      return EpicWeight;
	case ESpellRarity::Legendary: return LegendaryWeight;
	default:                      return CommonWeight;
	}
}

bool USpellGenerationSettings::AreElementsOpposed(const FGameplayTag& A, const FGameplayTag& B) const
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
