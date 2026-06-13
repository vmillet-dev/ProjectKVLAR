// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KitchenUnderPressure : ModuleRules
{
	public KitchenUnderPressure(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"DeveloperSettings",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"CommonUI",
			"CommonInput",
			"AudioModulation"
		]);

		PublicIncludePaths.AddRange([
			"KitchenUnderPressure",
			"KitchenUnderPressure/GameModes",
			"KitchenUnderPressure/Player",
			"KitchenUnderPressure/Character",
			"KitchenUnderPressure/Camera",
			"KitchenUnderPressure/System",
			"KitchenUnderPressure/Input",
			"KitchenUnderPressure/Interaction",
			"KitchenUnderPressure/Settings",
			"KitchenUnderPressure/Online",
			"KitchenUnderPressure/UI"
		]);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		PrivateDependencyModuleNames.AddRange(["OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemEOS"]);

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
