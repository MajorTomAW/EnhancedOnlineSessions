// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"

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
