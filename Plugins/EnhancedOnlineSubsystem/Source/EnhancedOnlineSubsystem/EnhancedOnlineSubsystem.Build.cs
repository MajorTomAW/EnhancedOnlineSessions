// Copyright © 2024 MajorT. All rights reserved.

using UnrealBuildTool;

public class EnhancedOnlineSubsystem : ModuleRules
{
	public EnhancedOnlineSubsystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemEOS",
			"OnlineSubsystemSteam"
		});
			
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{ 
			"CoreUObject",
			"Engine",
		});
	}
}
