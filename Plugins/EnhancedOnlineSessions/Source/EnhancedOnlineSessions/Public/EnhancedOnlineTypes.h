// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineKeyValuePair.h"

#include "EnhancedOnlineTypes.generated.h"

/**
 * An enumeration that represents the different types of authentication that can be used to log in.
 */
UENUM(BlueprintType)
enum class EEnhancedAuthType : uint8
{
	accountportal UMETA(DisplayName = "Account Portal"),
	developer UMETA(DisplayName = "Developer"),
	persistentauth UMETA(DisplayName = "Persistent Auth"),
	refreshtoken UMETA(DisplayName = "Refresh Token"),
	devicecode UMETA(DisplayName = "Device Code"),
	password UMETA(DisplayName = "Password"),
	externalauth UMETA(DisplayName = "External Auth"),
};

/**
 * Specifies the online features and connectivity that should be used for a game session.
 */
UENUM(BlueprintType)
enum class EEnhancedSessionOnlineMode : uint8
{
	Offline,
	LAN,
	Online,
};

/**
 * An enumeration that represents the possible login states of a player.
 */
UENUM(BlueprintType)
enum class EEnhancedLoginStatus : uint8
{
	/** Player has not logged in or chosen a local profile */
	NotLoggedIn,
	/** Player is using a local profile but is not logged in */
	UsingLocalProfile,
	/** Player has been validated by the platform specific authentication service */
	LoggedIn,
};

/**
 * A structure that stores extra settings that can be passed to the session on creation.
 */
USTRUCT(BlueprintType)
struct FEnhancedStoredExtraSessionSettings
{
	GENERATED_BODY()

	FEnhancedStoredExtraSessionSettings() = default;

	explicit FEnhancedStoredExtraSessionSettings(FName Key, FVariantData Data)
		: Key(Key)
		, Data(Data)
	{
	}

	/** The unique key that identifies the setting */
	FName Key;

	/** The data that is associated with the key */
	FVariantData Data;
};
