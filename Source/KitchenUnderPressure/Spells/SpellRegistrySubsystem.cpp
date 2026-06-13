// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellRegistrySubsystem.h"
#include "SpellConfig.h"
#include "SpellSettings.h"
#include "ElementDefinition.h"
#include "FormDefinition.h"
#include "ModifierDefinition.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

USpellRegistrySubsystem* USpellRegistrySubsystem::Get(const UObject* WorldContext)
{
	if (!GEngine || !WorldContext)
	{
		return nullptr;
	}
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<USpellRegistrySubsystem>();
		}
	}
	return nullptr;
}

void USpellRegistrySubsystem::EnsureBuilt()
{
	if (bBuilt)
	{
		return;
	}
	bBuilt = true;

	const USpellSettings* Settings = GetDefault<USpellSettings>();
	Config = Settings ? Settings->ActiveConfig.LoadSynchronous() : nullptr;
	if (!Config)
	{
		return;
	}

	for (const TSoftObjectPtr<UElementDefinition>& Soft : Config->ElementPool)
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

	for (const TSoftObjectPtr<UFormDefinition>& Soft : Config->FormPool)
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

	for (const TSoftObjectPtr<UModifierDefinition>& Soft : Config->ModifierPool)
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

const USpellConfig* USpellRegistrySubsystem::GetConfig()
{
	EnsureBuilt();
	return Config;
}

UElementDefinition* USpellRegistrySubsystem::FindElement(const FGameplayTag& Tag)
{
	EnsureBuilt();
	if (UElementDefinition** Found = ElementByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

UFormDefinition* USpellRegistrySubsystem::FindForm(const FGameplayTag& Tag)
{
	EnsureBuilt();
	if (UFormDefinition** Found = FormByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

UModifierDefinition* USpellRegistrySubsystem::FindModifier(const FGameplayTag& Tag)
{
	EnsureBuilt();
	if (UModifierDefinition** Found = ModifierByTag.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

const TArray<TObjectPtr<UElementDefinition>>& USpellRegistrySubsystem::GetLoadedElements()
{
	EnsureBuilt();
	return LoadedElements;
}

const TArray<TObjectPtr<UFormDefinition>>& USpellRegistrySubsystem::GetLoadedForms()
{
	EnsureBuilt();
	return LoadedForms;
}

const TArray<TObjectPtr<UModifierDefinition>>& USpellRegistrySubsystem::GetLoadedModifiers()
{
	EnsureBuilt();
	return LoadedModifiers;
}
