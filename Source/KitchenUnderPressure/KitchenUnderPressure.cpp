// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenUnderPressure.h"
#include "Modules/ModuleManager.h"
#include "AbilitySystemGlobals.h"

// Custom game module so GAS global data is initialised exactly once at startup. InitGlobalData() must
// run before the server ever replicates a GameplayCue or TargetData, otherwise GAS crashes the first
// time it serialises them. StartupModule runs identically in editor, PIE and packaged builds.
class FKitchenUnderPressureModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		UAbilitySystemGlobals::Get().InitGlobalData();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FKitchenUnderPressureModule, KitchenUnderPressure, "KitchenUnderPressure" );

DEFINE_LOG_CATEGORY(LogKitchenUnderPressure)