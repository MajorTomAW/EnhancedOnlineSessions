// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineMessageInterface.h"
#include "EnhancedOnlineTypes.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"
#include "Online/OnlineSessionNames.h"
#include "EnhancedOnlineRequests.generated.h"

class UEnhancedOnlineSubsystem;

/**
 * A session result object from the online subsystem that represents a joinable game session
 */
UCLASS(BlueprintType)
class UEnhancedSessionSearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** Pings the session to get the current ping in milliseconds */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Session")
	int32 GetPingInMs() const
	{
		return SearchResult.PingInMs;
	}

	/** Returns the maximum number of players that can join the session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Session")
	int32 GetMaxPlayers() const
	{
		return SearchResult.Session.SessionSettings.NumPublicConnections;
	}

	/** Returns the number of players currently in the session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Session")
	int32 GetCurrentPlayers() const
	{
		return SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
	}

	/** Returns the friendly name of the session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Session")
	FString GetServerFriendlyName() const
	{
		if (SearchResult.Session.SessionSettings.Settings.Array().Num() > 0)
		{
			for (auto& Setting : SearchResult.Session.SessionSettings.Settings)
			{
				if (Setting.Key == TEXT("FRIENDLYNAME"))
				{
					return Setting.Value.Data.ToString();
				}
			}
		}

		if (SearchResult.Session.OwningUserId.IsValid())
		{
			return SearchResult.Session.OwningUserId->ToString();
		}

		return TEXT("Unknown");
	}

public:
	/** The session id that uniquely identifies the session */
	FOnlineSessionSearchResult SearchResult;
};

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
	void InvalidateRequest()
	{
		OnlineSub = nullptr;
		UnbindDelegates();
		MarkAsGarbage();
	}

	/** Unbinds all delegates to this request */
	virtual void UnbindDelegates()
	{
		OnRequestFailed.RemoveAll(this);
		OnRequestFailed.Clear();
	}

	/** Constructs the request and sets up important data */
	virtual void ConstructRequest()
	{
		OnlineSub = Online::GetSubsystem(GetWorld());
		check(OnlineSub);
	}

	virtual void CompleteRequest()
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

	virtual void ConstructRequest() override
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

	virtual void UnbindDelegates() override
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
	
	/** The name of the matchmaking to specify what type of game mode this is */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString AdvertisementGameModeName;

	/** The friendly name of the session, used to display in the UI */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString SessionFriendlyName;

	/** The keyword that will be used to search for the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString SearchKeyword;
	
	/** Indicates if this request should create a player-hosted lobby if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseLobbiesIfAvailable;

	/** Indicates if this request should use voice chat if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseVoiceChatIfAvailable;

	/** A list of extra settings that will be stored in the session on creation */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	TArray<FEnhancedStoredExtraSessionSettings> StoredSettings;

	/** Indicates if the server should travel to the new session URL on success, or just open as normal level */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseServerTravelOnSuccess;

	/** The map that will be loaded when the session is created, this needs to be a valid Primary Asset top-level map */
	UPROPERTY(BlueprintReadWrite, Category = "Session", meta = (AllowedTypes = "World"))
	FPrimaryAssetId MapId;

	/** The maximum number of players that can join the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 MaxPlayerCount;

	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
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
	virtual FString ConstructTravelURL(bool bWithServerTravel = true) const
	{
		FString CombinedExtraArgs;

		if (OnlineMode == EEnhancedSessionOnlineMode::LAN)
		{
			CombinedExtraArgs += TEXT("?bIsLanMatch");
		}

		if (OnlineMode != EEnhancedSessionOnlineMode::Offline && bWithServerTravel)
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

/**
 * A request that is used to join an existing session.
 */
UCLASS()
class UEnhancedOnlineRequest_JoinSession : public UEnhancedOnlineRequest
{
	GENERATED_BODY()

public:
	/** The session that will be joined */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	UEnhancedSessionSearchResult* SessionToJoin;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

protected:
	friend UEnhancedOnlineSubsystem;
	IOnlineSessionPtr Sessions;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsSuccess, const TArray<UEnhancedSessionSearchResult*>&);
/**
 * A request that is used to search for sessions.
 */
UCLASS()
class UEnhancedOnlineRequest_SearchSessions : public UEnhancedOnlineRequest
{
	GENERATED_BODY()

public:
	/** Indicates for what type of session the search should be performed */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	EEnhancedSessionOnlineMode OnlineMode;

	/** Whether to search for player-hosted lobbies if the online subsystem does support that */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseLobbiesIfAvailable;

	/** Maximum number of search results that will be returned */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 MaxSearchResults;

	/** The keyword that will be used to search and filter for the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString SearchKeyword;

	/** List of all found sessions, this will be populated after the search is complete */
	UPROPERTY(BlueprintReadOnly, Category = "Session")
	TArray<TObjectPtr<UEnhancedSessionSearchResult>> SearchResults;

	/** Native delegate which is called when the search request is successful */
	FOnFindSessionsSuccess OnFindSessionsSuccess;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

	virtual void UnbindDelegates() override
	{
		Super::UnbindDelegates();
		
		OnFindSessionsSuccess.RemoveAll(this);
		OnFindSessionsSuccess.Clear();
	}

private:
	friend UEnhancedOnlineSubsystem;
	IOnlineSessionPtr Sessions;
};

/**
 * Helper class for garbage collection to ensure that the search request is properly referenced
 */
class FEnhancedOnlineSearchSettingBase : public FGCObject
{
public:
	FEnhancedOnlineSearchSettingBase(UEnhancedOnlineRequest_SearchSessions* InSearchRequest)
	{
		SearchRequest = InSearchRequest;
	}

	virtual ~FEnhancedOnlineSearchSettingBase() {}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SearchRequest);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FEnhancedOnlineSearchSettingBase");
		return NameString;
	}

public:
	TObjectPtr<UEnhancedOnlineRequest_SearchSessions> SearchRequest = nullptr;
};

/**
 * Helper class for search settings to be used with the online subsystem
 */
class FEnhancedOnlineSearchSettings : public FOnlineSessionSearch, public FEnhancedOnlineSearchSettingBase
{
public:
	FEnhancedOnlineSearchSettings(UEnhancedOnlineRequest_SearchSessions* InSearchRequest)
		: FEnhancedOnlineSearchSettingBase(InSearchRequest)
	{
		bIsLanQuery = (InSearchRequest->OnlineMode == EEnhancedSessionOnlineMode::LAN);
		MaxSearchResults = InSearchRequest->MaxSearchResults <= 0 ? INT_MAX : InSearchRequest->MaxSearchResults;
		PingBucketSize = 50;

		if (InSearchRequest->bUseLobbiesIfAvailable)
		{
			QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}

		if (!InSearchRequest->SearchKeyword.IsEmpty())
		{
			QuerySettings.Set(SEARCH_KEYWORDS, InSearchRequest->SearchKeyword, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FEnhancedOnlineSearchSettings() {}
};