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
