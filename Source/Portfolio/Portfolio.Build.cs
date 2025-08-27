// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Portfolio : ModuleRules
{
	public Portfolio(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] {"Portfolio"});
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "NavigationSystem", "GameplayAbilities", "GameplayTags", "GameplayTasks", "Niagara", "UMG" });


    }
}
