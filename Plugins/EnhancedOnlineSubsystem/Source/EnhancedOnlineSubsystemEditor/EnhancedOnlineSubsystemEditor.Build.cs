using UnrealBuildTool;

public class EnhancedOnlineSubsystemEditor : ModuleRules
{
    public EnhancedOnlineSubsystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UnrealEd",
                "OnlineSubsystem",
                "OnlineSubsystemEOS",
                "OnlineSubsystemUtils"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "SettingsEditor"
            }
        );
    }
}