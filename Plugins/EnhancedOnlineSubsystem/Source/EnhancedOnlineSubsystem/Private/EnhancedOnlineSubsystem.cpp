// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedOnlineSubsystem.h"

#define LOCTEXT_NAMESPACE "FEnhancedOnlineSubsystemModule"

DEFINE_LOG_CATEGORY(LogEnhancedSubsystem)

void FEnhancedOnlineSubsystemModule::StartupModule()
{
}

void FEnhancedOnlineSubsystemModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEnhancedOnlineSubsystemModule, EnhancedOnlineSubsystem)