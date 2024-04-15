// Some copyright should be here...

using UnrealBuildTool;

public class CommonOnlineSessions : ModuleRules
{
	public CommonOnlineSessions(ReadOnlyTargetRules Target) : base(Target)
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
