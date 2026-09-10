// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_A : ModuleRules
{
	public Project_A(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		if (Target.Type == TargetType.Editor)
		{
			// Editor-only: lets AbilityTagSync.cpp scan /Game/Abilities and register the standard
			// event tags for each Ability's AbilityName without manual Config/DefaultGameplayTags.ini edits.
			PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTagsEditor", "AssetRegistry" });
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
