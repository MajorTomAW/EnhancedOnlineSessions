// Copyright © 2024 Botanibots Team. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnhancedOnlineSubsystemSettings.generated.h"

/**
 * Enum to define the supported online subsystem used by the plugin.
 */
UENUM(BlueprintType)
enum class ESupportedSubsystem : uint8
{
	Steam UMETA(DisplayName = "Steam"),
	EOS UMETA(DisplayName = "Epic Online Services"),
	EOSPlus UMETA(DisplayName = "Epic Online Services Plus"),
};

/**
 * Settings to configure the online subsystem.
 */
UCLASS(Config = EnhancedOnlineSubsystemSettings)
class ENHANCEDONLINESUBSYSTEMEDITOR_API UEnhancedOnlineSubsystemSettings : public UObject
{
	GENERATED_BODY()

public:
	UEnhancedOnlineSubsystemSettings(const FObjectInitializer& ObjectInitializer);
	
	TMap<ESupportedSubsystem, TMap<FString, TMap<FString, FString>>> SubsystemSettings;

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	//~ End UObject Interface

	void UpdateSubsystemSettings();
	void LoadSubsystemSettings();

	static FString GetDefaultPlatformService(ESupportedSubsystem Subsystem);
	static FString GetNetDriverDefinition(ESupportedSubsystem Subsystem);

public:

	/** Settings */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	ESupportedSubsystem SupportedSubsystem;

	/** Enable or disable the online subsystem */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem")
	bool bIsEnabled;

	/** STEAM SETTINGS */
	
	/** The AppId to use for the Steam game */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	int32 SteamAppId;

	/** The Developer AppId to use for the Steam game */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	int32 SteamDevAppId;

	/** The port to use for the Steam game server */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	int GameServerQueryPort;

	/** Should the game relaunch in Steam if it's not correctly launched from Steam? */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	bool bRelaunchInSteam;
	
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	bool bVACEnabled;

	/** Enable or disable P2P packet relay */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	bool bAllowP2PPacketRelay;

	/** Display name for the game Version*/
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	FString GameVersion;

	/** Steam Net Driver Settings */
	UPROPERTY(VisibleDefaultsOnly, Category = "OnlineSubsystem|Steam", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::Steam"))
	FString NetDriverSteamSettings;

	/** EOS SETTINGS */
	UPROPERTY(EditAnywhere, Category = "OnlineSubsystem|EOS", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::EOS"))
	bool bIsUsingP2PSockets;

	UPROPERTY(VisibleDefaultsOnly, Category = "OnlineSubsystem|EOS", meta = (EditConditionHides, EditCondition = "SupportedSubsystem == ESupportedSubsystem::EOS"))
	FString NetDriverEOSSettings;
};
