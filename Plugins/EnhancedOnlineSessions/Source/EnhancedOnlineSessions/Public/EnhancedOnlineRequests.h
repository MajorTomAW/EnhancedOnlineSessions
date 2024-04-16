// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineMessageInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"
#include "EnhancedOnlineRequests.generated.h"

enum class EEnhancedSessionOnlineMode : uint8;
class UEnhancedOnlineSubsystem;
enum class EEnhancedAuthType : uint8;

/**
 * A delegate that is called when a request fails.
 *
 * @param Log		The log message that describes the failure.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRequestFailedWithLog, const FString& Log);

/**
 * A request object that can be used to interact with the online service.
 */
UCLASS(BlueprintType, Abstract)
class UEnhancedOnlineRequest : public UObject
{
	GENERATED_BODY()

public:
	/** The index of the local user that is making the request */
	UPROPERTY(BlueprintReadWrite, Category = "Request")
	int32 LocalUserIndex;

	/** Should the request be invalidated after it is completed */
	UPROPERTY(BlueprintReadWrite, Category = "Request")
	bool bInvalidateAfterComplete = false;
	
	/** Invalidates the request to save memory */
	FORCEINLINE void InvalidateRequest()
	{
		OnlineSub = nullptr;
		UnbindDelegates();
		MarkAsGarbage();
	}

	/** Unbinds all delegates to this request */
	FORCEINLINE virtual void UnbindDelegates()
	{
		OnRequestFailed.RemoveAll(this);
		OnRequestFailed.Clear();
	}

	/** Constructs the request and sets up important data */
	FORCEINLINE virtual void ConstructRequest()
	{
		OnlineSub = Online::GetSubsystem(GetWorld());
		check(OnlineSub);
	}

	FORCEINLINE virtual void CompleteRequest()
	{
		if (bInvalidateAfterComplete)
		{
			InvalidateRequest();	
		}
	}

	/** Native delegate which is called when the request fails */
	FOnRequestFailedWithLog OnRequestFailed;

protected:
	friend UEnhancedOnlineSubsystem;
	IOnlineSubsystem* OnlineSub;
};

/**
 * A delegate that is called when a login request is successful.
 *
 * @param LocalUserIndex	The index of the local user that logged in.
 * @param MessageId			The unique message id of the login request.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginUserSuccess, int32 LocalUserNum);

/**
 * A request that is used to log in to the online service.
 */
UCLASS()
class UEnhancedOnlineRequest_LoginUser : public UEnhancedOnlineRequest
{
	GENERATED_BODY()

public:
	/** The user id that will be used to log in */
	UPROPERTY(BlueprintReadWrite, Category = "Login")
	FString UserId;

	/** The user token that will be used to log in */
	UPROPERTY(BlueprintReadWrite, Category = "Login")
	FString UserToken;

	/** The type of authentication that will be used */
	UPROPERTY(BlueprintReadWrite, Category = "Login")
	EEnhancedAuthType AuthType;

	/** Native delegate which is called when the login request is successful */
	FOnLoginUserSuccess OnLoginSuccess;

	FORCEINLINE virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Identity = OnlineSub->GetIdentityInterface();
		check(Identity);

		Identity->AddOnLoginCompleteDelegate_Handle(LocalUserIndex, FOnLoginCompleteDelegate::CreateLambda(
			[this] (int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
			{
				if (bWasSuccessful)
				{
					OnLoginSuccess.Broadcast(LocalUserNum);
					CompleteRequest();
				}
				else
				{
					OnRequestFailed.Broadcast(Error);
					InvalidateRequest();
				}
			}));
	}

	FORCEINLINE virtual void UnbindDelegates() override
	{
		Super::UnbindDelegates();

		OnLoginSuccess.RemoveAll(this);
		OnLoginSuccess.Clear();
	}

protected:
	friend UEnhancedOnlineSubsystem;
	IOnlineIdentityPtr Identity;
};

/**
 * A request that is used to create a new session.
 */
UCLASS()
class UEnhancedOnlineRequest_CreateSession : public UEnhancedOnlineRequest
{
	GENERATED_BODY()

public:
	/** Indicates if the session is a full online session or a different type */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	EEnhancedSessionOnlineMode OnlineMode;

	/** Indicates if this request should create a player-hosted lobby if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseLobbiesIfAvailable;

	/** The name of the matchmaking to specify what type of game mode this is */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString AdvertisementGameModeName;

	/** The map that will be loaded when the session is created, this needs to be a valid Primary Asset top-level map */
	UPROPERTY(BlueprintReadWrite, Category = "Session", meta = (AllowedTypes = "World"))
	FPrimaryAssetId MapId;

	/** The maximum number of players that can join the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 MaxPlayerCount;

	FORCEINLINE virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);

		//TODO:
	}

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const {return MaxPlayerCount; }

	/** Returns the full map name that will be loaded when the session is created */
	virtual FString GetFullMapName() const
	{
		FAssetData MapAssetData;
		if (UAssetManager::Get().GetPrimaryAssetData(MapId, MapAssetData))
		{
			return MapAssetData.AssetName.ToString();
		}
		return FString();
	}

	/** Constructs the full travel URL that will be used to join the session */
	virtual FString ConstructTravelURL() const
	{
		FString CombinedExtraArgs;

		if (OnlineMode == EEnhancedSessionOnlineMode::LAN)
		{
			CombinedExtraArgs += TEXT("?bIsLanMatch");
		}

		if (OnlineMode != EEnhancedSessionOnlineMode::Offline)
		{
			CombinedExtraArgs += TEXT("?listen");
		}

		return FString::Printf(TEXT("%s%s"),
			*GetFullMapName(),
			*CombinedExtraArgs);
	}

	/** Validates the request and logs any errors that are found */
	virtual bool ValidateAndLogErrors(FText& OutError) const
	{
#if WITH_SERVER_CODE
		if (GetFullMapName().IsEmpty())
		{
			OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed."), FText::FromString(MapId.ToString()));
			return false;
		}
		return true;
#else
		return false;
#endif
	}

protected:
	friend UEnhancedOnlineSubsystem;
	IOnlineSessionPtr Sessions;
};
