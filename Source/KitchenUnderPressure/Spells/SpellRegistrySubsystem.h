// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SpellRegistrySubsystem.generated.h"

class USpellConfig;
class UElementDefinition;
class UFormDefinition;
class UModifierDefinition;

/**
 * Runtime registry for spell content. Loads the active USpellConfig (selected by USpellSettings) and
 * its element/form/modifier pools once, then resolves a spell's tags back to the authored data assets.
 * Keeps this runtime state in a subsystem instead of on a settings CDO (the previous approach).
 * The loaded definitions are kept alive by the hard-ref arrays below.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API USpellRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Resolves the subsystem from any world context object (null-safe).
	static USpellRegistrySubsystem* Get(const UObject* WorldContext);

	// Active config holding the pools and tuning (loads on first use; null if none is assigned).
	const USpellConfig* GetConfig();

	UElementDefinition* FindElement(const FGameplayTag& Tag);
	UFormDefinition* FindForm(const FGameplayTag& Tag);
	UModifierDefinition* FindModifier(const FGameplayTag& Tag);

	const TArray<TObjectPtr<UElementDefinition>>& GetLoadedElements();
	const TArray<TObjectPtr<UFormDefinition>>& GetLoadedForms();
	const TArray<TObjectPtr<UModifierDefinition>>& GetLoadedModifiers();

private:
	void EnsureBuilt();

	UPROPERTY()
	TObjectPtr<USpellConfig> Config;

	UPROPERTY()
	TArray<TObjectPtr<UElementDefinition>> LoadedElements;

	UPROPERTY()
	TArray<TObjectPtr<UFormDefinition>> LoadedForms;

	UPROPERTY()
	TArray<TObjectPtr<UModifierDefinition>> LoadedModifiers;

	TMap<FGameplayTag, UElementDefinition*> ElementByTag;
	TMap<FGameplayTag, UFormDefinition*> FormByTag;
	TMap<FGameplayTag, UModifierDefinition*> ModifierByTag;

	bool bBuilt = false;
};
