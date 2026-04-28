// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BeekeepingSim : ModuleRules
{
	public BeekeepingSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BeekeepingSim",
			"BeekeepingSim/Variant_Horror",
			"BeekeepingSim/Variant_Horror/UI",
			"BeekeepingSim/Variant_Shooter",
			"BeekeepingSim/Variant_Shooter/AI",
			"BeekeepingSim/Variant_Shooter/UI",
			"BeekeepingSim/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
