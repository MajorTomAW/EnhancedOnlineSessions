#include "EnhancedOnlineSubsystemEditor.h"

#include "ISettingsModule.h"
#include "Settings/EnhancedOnlineSubsystemSettings.h"

#define LOCTEXT_NAMESPACE "FEnhancedOnlineSubsystemEditorModule"

void FEnhancedOnlineSubsystemEditorModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "EnhancedOnlineSubsystemSettings",
			LOCTEXT("RuntimeSettingsName", "Online Enhanced Subsystem"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the EnhancedOnlineSubsystem plugin"),
			GetMutableDefault<UEnhancedOnlineSubsystemSettings>());
	}
}

void FEnhancedOnlineSubsystemEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "EnhancedOnlineSubsystemSettings");
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FEnhancedOnlineSubsystemEditorModule, EnhancedOnlineSubsystemEditor)