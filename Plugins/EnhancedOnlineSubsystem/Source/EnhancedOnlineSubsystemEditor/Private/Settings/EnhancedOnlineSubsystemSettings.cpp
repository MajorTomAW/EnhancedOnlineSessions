// Copyright © 2024 Botanibots Team. All rights reserved.


#include "Settings/EnhancedOnlineSubsystemSettings.h"

#include "ISettingsEditorModule.h"
#include "Misc/ConfigCacheIni.h"

UEnhancedOnlineSubsystemSettings::UEnhancedOnlineSubsystemSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedSubsystem = ESupportedSubsystem::Steam;
	bIsEnabled = true;

	/** STEAM SETTINGS */
	SteamAppId = 480;
	SteamDevAppId = 480;
	GameServerQueryPort = 27015;
	bRelaunchInSteam = true;
	bVACEnabled = true;
	bAllowP2PPacketRelay = true;
	GameVersion = TEXT("1.0.0.0");
	NetDriverSteamSettings = GetNetDriverDefinition(ESupportedSubsystem::Steam);

	/** EOS SETTINGS */
	bIsUsingP2PSockets = false;
	NetDriverEOSSettings = GetNetDriverDefinition(ESupportedSubsystem::EOS);

	/** Platform specific settings Steam */
	UpdateSubsystemSettings();

	// Load the settings from the config file if they exist
	for (auto& SubsystemSetting : SubsystemSettings)
	{
		for (auto& Setting : SubsystemSetting.Value)
		{
			for (auto& Pair : Setting.Value)
			{
				FString Value;
				if (GConfig->GetString(
					*Setting.Key,
					*Pair.Key,
					Value,
					FConfigCacheIni::NormalizeConfigIniPath(FPaths::SourceConfigDir() / TEXT("DefaultEngine.ini"))))
				{
					Pair.Value = Value;
					UE_LOG(LogTemp, Log, TEXT("Found Key: %s, Value: %s"), *Pair.Key, *Pair.Value);
				}
			}
		}
	}

	LoadSubsystemSettings();
}

void UEnhancedOnlineSubsystemSettings::UpdateSubsystemSettings()
{
	/** STEAM */
	TMap<FString, TMap<FString, FString>> SubsystemSteamSettings;
	
	SubsystemSteamSettings.Add(TEXT("OnlineSubsystem"), TMap<FString, FString>{
		{ TEXT("DefaultPlatformService"), GetDefaultPlatformService(SupportedSubsystem) },
	});
	
	SubsystemSteamSettings.Add(TEXT("OnlineSubsystemSteam"), TMap<FString, FString>{
		{ TEXT("bEnabled"), bIsEnabled ? TEXT("True") : TEXT("False") },
		{ TEXT("SteamAppId"), FString::FromInt(SteamAppId) },
		{ TEXT("SteamDevAppId"), FString::FromInt(SteamDevAppId) },
		{ TEXT("GameServerQueryPort"), FString::FromInt(GameServerQueryPort) },
		{ TEXT("bRelaunchInSteam"), bRelaunchInSteam ? TEXT("True") : TEXT("False") },
		{ TEXT("bVACEnabled"), bVACEnabled ? TEXT("True") : TEXT("False") },
		{ TEXT("bAllowP2PPacketRelay"), bAllowP2PPacketRelay ? TEXT("True") : TEXT("False") },
		{ TEXT("GameVersion"), GameVersion },
	});

	SubsystemSteamSettings.Add(TEXT("/Script/OnlineSubsystemSteam.SteamNetDriver"), TMap<FString, FString>{
		{ TEXT("NetConnectionClassName"), TEXT("OnlineSubsystemSteam.SteamNetConnection") },
		{ TEXT("AllowDownloads"), TEXT("True") },
	});

	SubsystemSteamSettings.Add(TEXT("/Script/Engine.GameEngine"), TMap<FString, FString>{
		{ TEXT("+NetDriverDefinitions"), NetDriverSteamSettings },
	});

	/** EOS */
	TMap<FString, TMap<FString, FString>> SubsystemEOSSettings;
	
	SubsystemEOSSettings.Add(TEXT("OnlineSubsystem"), TMap<FString, FString>{
		{ TEXT("DefaultPlatformService"), GetDefaultPlatformService(SupportedSubsystem) },
	});
	
	SubsystemEOSSettings.Add(TEXT("OnlineSubsystemEOS"), TMap<FString, FString>{
		{ TEXT("bEnabled"), bIsEnabled ? TEXT("True") : TEXT("False") },
	});

	SubsystemEOSSettings.Add(TEXT("/Script/SocketSubsystemEOS.NetDriverEOSBase"), TMap<FString, FString>{
		{ TEXT("bIsUsingP2PSockets"), bIsUsingP2PSockets ? TEXT("True") : TEXT("False") },
	});

	SubsystemEOSSettings.Add(TEXT("/Script/Engine.GameEngine"), TMap<FString, FString>{
		{ TEXT("+NetDriverDefinitions"), NetDriverEOSSettings },
	});

	SubsystemSettings.Add(ESupportedSubsystem::EOS, SubsystemEOSSettings);
	SubsystemSettings.Add(ESupportedSubsystem::Steam, SubsystemSteamSettings);
}

void UEnhancedOnlineSubsystemSettings::LoadSubsystemSettings()
{
	// Load the settings from the SubsystemSettings map and apply them to the class properties
	bIsEnabled = SubsystemSettings[ESupportedSubsystem::EOS][TEXT("OnlineSubsystemEOS")][TEXT("bEnabled")].Equals(TEXT("True")) || SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("bEnabled")].Equals(TEXT("True"));
	
	SteamAppId = FCString::Atoi(*SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("SteamAppId")]);
	SteamDevAppId = FCString::Atoi(*SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("SteamDevAppId")]);
	GameServerQueryPort = FCString::Atoi(*SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("GameServerQueryPort")]);
	bRelaunchInSteam = SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("bRelaunchInSteam")].Equals(TEXT("True"));
	bVACEnabled = SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("bVACEnabled")].Equals(TEXT("True"));
	bAllowP2PPacketRelay = SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("bAllowP2PPacketRelay")].Equals(TEXT("True"));
	GameVersion = SubsystemSettings[ESupportedSubsystem::Steam][TEXT("OnlineSubsystemSteam")][TEXT("GameVersion")];

	bIsUsingP2PSockets = SubsystemSettings[ESupportedSubsystem::EOS][TEXT("/Script/SocketSubsystemEOS.NetDriverEOSBase")][TEXT("bIsUsingP2PSockets")].Equals(TEXT("True"));

	// Load the Supported Subsystem from the EnhancedOnlineSubsystemSettings using GConfig
	FString DefaultEngineIni = FPaths::SourceConfigDir() / TEXT("DefaultEngine.ini");
	FConfigCacheIni::NormalizeConfigIniPath(DefaultEngineIni);

	FString DefaultPlatformService;
	GConfig->GetString(
		TEXT("/Script/EnhancedOnlineSubsystemEditor.EnhancedOnlineSubsystemSettings"),
		TEXT("SupportedSubsystem"),
		DefaultPlatformService,
		DefaultEngineIni);

	// Set the SupportedSubsystem to the value from the config file
	SupportedSubsystem = static_cast<ESupportedSubsystem>(FCString::Atoi(*DefaultPlatformService));
}


void UEnhancedOnlineSubsystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (!GConfig)
	{
		return;
	}

	if (PropertyChangedEvent.Property)
	{
		UEnhancedOnlineSubsystemSettings* Settings = GetMutableDefault<UEnhancedOnlineSubsystemSettings>();
		FString DefaultEngineIni = FPaths::SourceConfigDir() / TEXT("DefaultEngine.ini");
		FConfigCacheIni::NormalizeConfigIniPath(DefaultEngineIni);

		UpdateSubsystemSettings();

		/** Remove redundant settings */
		for (auto& SubsystemSetting : SubsystemSettings)
		{
			if (SubsystemSetting.Key != SupportedSubsystem)
			{
				for (auto& Setting : SubsystemSetting.Value)
				{
					for (auto& Pair : Setting.Value)
					{
						GConfig->RemoveKey(
						*Setting.Key,
						*Pair.Key,
						DefaultEngineIni);
					}
				}
			}
		}

		/** Apply settings */
		for (auto& SubsystemSetting : SubsystemSettings)
		{
			if (SubsystemSetting.Key == SupportedSubsystem)
			{
				for (auto& Setting : SubsystemSetting.Value)
				{
					for (auto& Pair : Setting.Value)
					{
						GConfig->SetString(
							*Setting.Key,
							*Pair.Key,
							*Pair.Value,
							DefaultEngineIni);
					}
				}	
			}
		}

		GConfig->Flush(false, DefaultEngineIni);
			
		Settings->SaveConfig(CPF_Config, *DefaultEngineIni);

		UE_LOG(LogTemp, Warning, TEXT("Property changed, saving to: %s"), *DefaultEngineIni);

		ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor");
		if (SettingsEditorModule)
		{
			SettingsEditorModule->OnApplicationRestartRequired();
		}
	}
}

void UEnhancedOnlineSubsystemSettings::PostLoad()
{
	UObject::PostLoad();
}


FString UEnhancedOnlineSubsystemSettings::GetDefaultPlatformService(ESupportedSubsystem Subsystem)
{
	switch (Subsystem)
	{
	case ESupportedSubsystem::Steam:
		return TEXT("Steam");
	case ESupportedSubsystem::EOS:
		return TEXT("EOS");
	case ESupportedSubsystem::EOSPlus:
		return TEXT("EOSPlus");
	default:
		return TEXT("Steam");
	}
}

FString UEnhancedOnlineSubsystemSettings::GetNetDriverDefinition(ESupportedSubsystem Subsystem)
{
	switch (Subsystem)
	{
	case ESupportedSubsystem::Steam:
		return TEXT("(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemSteam.SteamNetDriver\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")");
	case ESupportedSubsystem::EOS:
		return TEXT("(DefName=\"GameNetDriver\",DriverClassName=\"OnlineSubsystemEOS.NetDriverEOS\",DriverClassNameFallback=\"OnlineSubsystemUtils.IpNetDriver\")");
	case ESupportedSubsystem::EOSPlus:
		return TEXT("GameNetDriver EOSPlusNetDriver");
	default:
		return TEXT("GameNetDriver SteamNetDriver");
	}
}
