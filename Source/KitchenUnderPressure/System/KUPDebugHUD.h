// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "KUPDebugHUD.generated.h"

struct FSpellDefinition;

/**
 * Prototype debug overlay, on by default (toggle with the "KUP.Debug.HUD" console variable; compiled
 * out of Shipping builds). Draws a health bar above every living enemy plus a bottom-left panel with
 * the local player's health and both equipped spells, so a run is readable without any authored UI.
 * Set as the game mode's HUD class; real UMG widgets can replace it later.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API AKUPDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
//TODO#if !UE_BUILD_SHIPPING
	void DrawEnemyHealthBars();
	void DrawLocalPlayerPanel();

	/** One readable line for an equipped spell, e.g. "Fire Projectile [Rare] x1.4 +Overcharged". */
	static FString DescribeSpell(const FSpellDefinition& Spell);
//TODO#endif
};
