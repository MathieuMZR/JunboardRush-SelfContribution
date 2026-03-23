// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class JunkBoardRush : ModuleRules
{
	public JunkBoardRush(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Niagara", "HTTP", "Json", "JsonUtilities" });

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		PublicDependencyModuleNames.AddRange(new string[] {"OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam"});
	}
}
