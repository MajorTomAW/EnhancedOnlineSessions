// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemUtilsModule.h"

#include "CommonOnlineTypes.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRequestFailedWithLog, const FString& Log);
DECLARE_MULTICAST_DELEGATE(FOnEmptyRequestSuccess)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEmptyIndexRequestSuccess, int32 LocalPlayerIndex);

DECLARE_DYNAMIC_DELEGATE(FBlueprintOnEmptyRequestSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnEmptyIndexRequestSuccess, int32, LocalPlayerIndex);

/** A key-value pair that can be passed to a session */
USTRUCT(BlueprintType)
struct FSessionPropertyKeyPair
{
	GENERATED_BODY()

	FName Key;
	FVariantData Data;
};

/** A list of key-value pairs that can be passed to a session */
USTRUCT(BlueprintType)
struct FStoredSessionSettings
{
	GENERATED_BODY()

	FStoredSessionSettings() {};
	explicit FStoredSessionSettings(const TArray<FSessionPropertyKeyPair>& InStoredSettings) : StoredSettings(InStoredSettings) {};
	TArray<FSessionPropertyKeyPair> StoredSettings;
};

/** Specifies the type of authentication to use when logging in */
UENUM(BlueprintType)
enum class EAuthType : uint8
{
	accountportal UMETA(DisplayName = "Account Portal"),
	developer UMETA(DisplayName = "Developer"),
	persistentauth UMETA(DisplayName = "Persistent Auth"),
	refreshtoken UMETA(DisplayName = "Refresh Token"),
	devicecode UMETA(DisplayName = "Device Code"),
	password UMETA(DisplayName = "Password"),
	externalauth UMETA(DisplayName = "External Auth"),
};

/** Specifies the online features and connectivity that should be used for a game session */
UENUM(BlueprintType)
enum class ECommonSessionOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

/** Specifies the current online presence state of a friend */
UENUM(BlueprintType)
enum class ECommonFriendOnlineSate : uint8
{
	Online,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat
};

/** Specifies the current online presence state filter of a friend */
UENUM(BlueprintType)
enum class ECommonFriendOnlineSateFilter : uint8
{
	Online,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat,
	All
};

/** Blueprint exposed online friend presence info */
USTRUCT(BlueprintType)
struct FCommonOnlineFriendPresenceInfo
{
	GENERATED_BODY()

	FCommonOnlineFriendPresenceInfo()
		: bIsOnline(false)
		, bIsPlaying(false)
		, bIsPlayingThisGame(false)
		, bIsJoinable(false)
		, bHasVoiceSupport(false)
		, StatusString(TEXT(""))
	{
	};

public:
	/** Whether this friend is online or not */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	uint8 bIsOnline:1;

	/** The current playing status of this friend */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	uint8 bIsPlaying:1;

	/** Whether this friend is playing the same game or not */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	uint8 bIsPlayingThisGame:1;

	/** Whether this friend is in a joinable session or not */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	uint8 bIsJoinable:1;

	/** Whether this friend is able to use voice chat or not */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	uint8 bHasVoiceSupport:1;

	/** Extra data about this friend's presence */
	UPROPERTY(BlueprintReadOnly, Category = "Presence")
	FString StatusString;
};

/** Blueprint exposed online friend struct */
USTRUCT(BlueprintType)
struct FCommonOnlineFriendInfo
{
	GENERATED_BODY()

	FCommonOnlineFriendInfo() = default;

public:
	/** Display- or Nickname of this friend */
	UPROPERTY(BlueprintReadOnly, Category = "Friend")
	FString DisplayName;

	/** Real name of this friend, if available */
	UPROPERTY(BlueprintReadOnly, Category = "Friend")
	FString RealName;

	/** Online state of this friend */
	UPROPERTY(BlueprintReadOnly, Category = "Friend")
	ECommonFriendOnlineSate OnlineState;

	/** Friend presence info */
	UPROPERTY(BlueprintReadOnly, Category = "Friend")
	FCommonOnlineFriendPresenceInfo PresenceInfo;
};

/** Detailed information about the online error */
USTRUCT(BlueprintType)
struct FOnlineResultInfo
{
	GENERATED_BODY()

	/** Whether the operation was successful or not. If it was successful, the error message will be empty */
	UPROPERTY(BlueprintReadOnly, Category = "Error")
	bool bWasSuccessful = true;

	/** The unique error id */
	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString ErrorId;

	/** Detailed error message */
	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString ErrorMessage;

	void FromOnlineError(const FOnlineError& OnlineError);
};
