// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OptimizedGASDemo : ModuleRules
{
	public OptimizedGASDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"OptimizedGASDemo",
			"OptimizedGASDemo/Variant_Platforming",
			"OptimizedGASDemo/Variant_Platforming/Animation",
			"OptimizedGASDemo/Variant_Combat",
			"OptimizedGASDemo/Variant_Combat/AI",
			"OptimizedGASDemo/Variant_Combat/Animation",
			"OptimizedGASDemo/Variant_Combat/Gameplay",
			"OptimizedGASDemo/Variant_Combat/Interfaces",
			"OptimizedGASDemo/Variant_Combat/UI",
			"OptimizedGASDemo/Variant_SideScrolling",
			"OptimizedGASDemo/Variant_SideScrolling/AI",
			"OptimizedGASDemo/Variant_SideScrolling/Gameplay",
			"OptimizedGASDemo/Variant_SideScrolling/Interfaces",
			"OptimizedGASDemo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
