// Copyright © 2024 MajorT. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EnhancedSubsystemTarget : TargetRules
{
	public EnhancedSubsystemTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "EnhancedSubsystem" } );
	}
}
