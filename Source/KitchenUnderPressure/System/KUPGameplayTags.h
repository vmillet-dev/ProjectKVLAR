// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Native gameplay tags for the Alchemist spell and combat systems. Declared in C++ (not in
 * DefaultGameplayTags.ini) so every tag is compile-checked and refactor-safe at its call sites,
 * matching the project's "typed config over loose strings" approach. Add new elements, forms and
 * modifiers here as the spell system grows.
 */
namespace KUPTags
{
	// --- Elements (spell layer 1: damage type, colour, status) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Fire);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Ice);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Poison);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Lightning);

	// --- Forms (spell layer 2: how the spell is delivered) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Form_Projectile);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Form_Nova);

	// --- Modifiers (spell layer 3: extra behaviours carried by rare+ spells) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Modifier_Overcharged);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Modifier_Unstable);

	// --- Ability activation / input routing (one tag per hand) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Cast_Left);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Cast_Right);

	// --- Character states ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);

	// --- SetByCaller magnitudes (keys for data-driven GameplayEffect magnitudes) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);

	// --- Status effects (identity tags granted by burn / freeze / poison effects) ---
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Burn);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Freeze);
	KITCHENUNDERPRESSURE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Poisoned);
}
