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
 * Specifies the online presence state of a player
 */
UENUM(BlueprintType)
enum class EBlueprintEnhancedPresenceState : uint8
{
	Online,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat,
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

/**
 * Blueprint exposed struct for the enhanced friend presence info
 */
USTRUCT(BlueprintType)
struct FEnhancedOnlineFriendPresenceInfo
{
	GENERATED_BODY()

public:
	/** Whether the friend is online or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	uint8 bIsOnline:1 = false;

	/** Whether the friend is playing a game or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	uint8 bIsPlaying:1 = false;

	/** Whether the friend is playing the same game or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	uint8 bIsPlayingThisGame:1 = false;

	/** Whether the friend is in a joinable session or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	uint8 bIsJoinable:1 = false;

	/** Whether the friend is able to use voice chat or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	uint8 bHasVoiceSupport:1 = false;

	/** The online presence state of the friend */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	EBlueprintEnhancedPresenceState PresenceState = EBlueprintEnhancedPresenceState::Offline;

	/** The additional status string of the friend */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Presence Info")
	FString StatusString;
};

/**
 * Blueprint exposed struct for the enhanced friend info
 */
USTRUCT(BlueprintType)
struct FEnhancedBlueprintFriendInfo
{
	GENERATED_BODY()

public:
	/** The display name of the friend */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Info")
	FString DisplayName;

	/** The real name of the friend. Can often be empty */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Info")
	FString RealName;

	/** The unique net id of the friend */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Info")
	FUniqueNetIdRepl UniqueNetId;

	/** The online presence info of the friend */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Friend Info")
	FEnhancedOnlineFriendPresenceInfo PresenceInfo;
};
