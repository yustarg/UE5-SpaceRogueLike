// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VibeCoding : ModuleRules
{
	public VibeCoding(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"Niagara",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Add include paths for organized folder structure
		PublicIncludePaths.AddRange([
			ModuleDirectory + "/Player",
			ModuleDirectory + "/Enemy",
			ModuleDirectory + "/Core",
			ModuleDirectory + "/Weapons",
			ModuleDirectory + "/GameplayAbilities"
		]);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
