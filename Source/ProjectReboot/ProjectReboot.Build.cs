// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectReboot : ModuleRules
{
	public ProjectReboot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"GameplayStateTreeModule",
			"StateTreeModule",
			"AIModule",
			"NavigationSystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities",
			"GameplayTags",
			"AnimGraphRuntime",
			"RogueliteCore",
			"RogueliteGAS",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayAbilities",
			"GameplayTasks", "Niagara", "MotionWarping",

		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
