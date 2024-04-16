// Copyright © 2024 MajorT. All rights reserved.

using UnrealBuildTool;

public class EnhancedOnlineSessions : ModuleRules
{
	public EnhancedOnlineSessions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreOnline",
				"OnlineSubsystem",
				"OnlineSubsystemUtils"
			});
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ApplicationCore"
			});
	}
}
