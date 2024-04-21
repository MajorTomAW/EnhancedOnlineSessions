// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "EnhancedOnlineTypes.generated.h"

#define MaxNumConnectionsSession 1000
#define MaxNumConnectionsLobby 64

#define SETTING_FRIENDLYNAME FName(TEXT("FRIENDLYNAME"))

/**
 * Specifies the online mode of a game session
 */
UENUM(BlueprintType)
enum class EEnhancedSessionOnlineMode : uint8
{
	Online,
	Offline,
	LAN,
};

/**
 * Specifies the authentication type for the enhanced login system
 */
UENUM(BlueprintType)
enum class EEnhancedLoginAuthType : uint8
{
	AccountPortal,
	Developer,
	PersistentAuth,
	RefreshToken,
	DeviceCode,
	ExternalAuth,
	Password,
};

/**
 * Helper class for the online session settings
 */
class FEnhancedOnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FEnhancedOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4, bool bShouldAllowJoinInProgress = true)
	{
		NumPublicConnections = MaxNumPlayers;
		NumPrivateConnections = 0;
		bIsLANMatch = bIsLAN;
		bShouldAdvertise = true;
		bAllowJoinInProgress = bShouldAllowJoinInProgress;
		bAllowInvites = true;
		bUsesPresence = bIsPresence;
		bAllowJoinViaPresence = bIsPresence;
		bAllowJoinViaPresenceFriendsOnly = false;
		bUsesStats = true;
		bAntiCheatProtected = false;
		bIsDedicated = false;
	}
	virtual ~FEnhancedOnlineSessionSettings() {}
};