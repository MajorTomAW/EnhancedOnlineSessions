// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedOnlineSessions.h"

#define LOCTEXT_NAMESPACE "FEnhancedOnlineSessionsModule"

DEFINE_LOG_CATEGORY(LogEnhancedSubsystem)

void FEnhancedOnlineSessionsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FEnhancedOnlineSessionsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEnhancedOnlineSessionsModule, EnhancedOnlineSessions)