// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedOnlineTypes.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"
#include "Online/OnlineSessionNames.h"
#include "FindSessionsCallbackProxy.h"
#include "EnhancedOnlineRequests.generated.h"

enum class EEnhancedSessionOnlineMode : uint8;
class UEnhancedOnlineSessionsSubsystem;

/**
 * Delegate for when a request failed
 * @param Reason	The reason why the request failed
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRequestFailedWithLog, const FString& /* Reason */);

/**
 * Base request class that manages garbage collection, used to communicate with the online service
 */
UCLASS(BlueprintType, Abstract)
class UEnhancedOnlineRequestBase : public UObject
{
	GENERATED_BODY()

public:
	//~ Being UEnhancedOnlineRequestBase Interface
	virtual void ConstructRequest()
	{
		OnlineSub = Online::GetSubsystem(GetWorld());
		check(OnlineSub);
	}
	
	virtual void InvalidateRequest()
	{
		if (OnRequestFailedDelegate.IsBound())
		{
			OnRequestFailedDelegate.RemoveAll(this);
			OnRequestFailedDelegate.Clear();

			MarkAsGarbage();
		}
	}

	virtual void CompleteRequest()
	{
		if (bInvalidateOnCompletion)
		{
			InvalidateRequest();
		}
	}
	//~ End UEnhancedOnlineRequestBase Interface

	/** Should the request be garbage collected when it's completed */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bInvalidateOnCompletion;

	/** The index of the local user who made the request */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	int32 LocalUserIndex;

	/** Native delegate for when the request fails */
	FOnRequestFailedWithLog OnRequestFailedDelegate;

protected:
	friend UEnhancedOnlineSessionsSubsystem;

	/** Online subsystem pointer */
	IOnlineSubsystem* OnlineSub;
};

/**
 * Delegate for when a session is created
 * @param LocalUserIndex	The index of the local user who created the session request
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionCompleted, int32 /* Local User Index */ , const FName /* Session Name */);

/*
 * Base session request class, used to create a session
 */
UCLASS()
class UEnhancedOnlineRequest_Session : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	//~ Begin UEnhancedOnlineRequestBase Interface
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

	virtual void InvalidateRequest() override
	{
		if (OnCreateSessionCompleted.IsBound())
		{
			OnCreateSessionCompleted.RemoveAll(this);
			OnCreateSessionCompleted.Clear();
		}
		
		Super::InvalidateRequest();
	}
	//~ End UEnhancedOnlineRequestBase Interface


	/** Indicates the online mode of the session */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	EEnhancedSessionOnlineMode OnlineMode;

	/** The maximum number of players that can join the session */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	int32 MaxPlayerCount;

	/** The friendly name of the matchmaking game session */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString GameModeAdvertisementName;

	/** The friendly name of the session which will be displayed in the UI */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString FriendlyName;

	/** Keyword used to find the session, multiple sessions can share the same keyword */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString SearchKeyword;

	/** Whether the session is a lobby */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bUseLobbiesIfAvailable;

	/** Whether to use voice chat if available */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bUseVoiceChatIfAvailable;

	/** Should the session use presence */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bUsesPresence;

	/** Whether to allow players to join while the session is in progress */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bAllowJoinInProgress;
	
	/** Native delegate for when the session is created */
	FOnCreateSessionCompleted OnCreateSessionCompleted;

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const { return MaxPlayerCount; }

	/** Returns the travel URL that will be used to load the map when the session is created */
	virtual FURL GetTravelURL() const
	{
		FURL TravelURL;

		if (OnlineMode == EEnhancedSessionOnlineMode::LAN)
		{
			TravelURL.Op.Add(TEXT("?bIsLanMatch"));
		}

		if (OnlineMode != EEnhancedSessionOnlineMode::Offline)
		{
			TravelURL.Op.Add(TEXT("?listen"));
		}

		return TravelURL;
	}

	/** Validates the request and logs any errors */
	virtual FString GetMapName() const { return FString(); }

	/** Validates the request and logs any errors */
	virtual bool ValidateAndLogErrors(FText& OutError) const
	{
#if WITH_SERVER_CODE
		if (GetMapName().IsEmpty())
		{
			OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormant", "Can't find the asset data for MapId {0}"), FText::FromString(GetMapName()));
			return false;
		}
		return true;
#else
		return false;
#endif
	}

protected:
	friend UEnhancedOnlineSessionsSubsystem;

	/** Online session pointer */
	IOnlineSessionPtr Sessions;
};

/**
 * Request class used to create an online session
 */
UCLASS()
class UEnhancedOnlineRequest_CreateSession : public UEnhancedOnlineRequest_Session
{
	GENERATED_BODY()

public:
	/** The map of the session which will be loaded when the session is created */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request", meta = (AllowedTypes = "World"))
	FPrimaryAssetId MapId;

	/** Whether the session is dedicated */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bIsDedicated;

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const override
	{
		return MaxPlayerCount <= 0 ? MaxNumConnectionsSession : MaxPlayerCount;
	}

	/** Returns the full map name that will be loaded when the session is created */
	virtual FString GetMapName() const override
	{
		FAssetData MapAssetData;
		if (UAssetManager::Get().GetPrimaryAssetData(MapId, MapAssetData))
		{
			return MapAssetData.AssetName.ToString();
		}
		return FString();
	}

	/** Returns the travel URL of the map id */
	virtual FURL GetTravelURL() const override
	{
		FURL TravelURL = Super::GetTravelURL();
		TravelURL.Map = GetMapName();

		return TravelURL;
	}
};

/**
 * Request class used to start an online session
 */
UCLASS()
class UEnhancedOnlineRequest_StartSession : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	//~ Begin UEnhancedOnlineRequestBase Interface
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

	virtual void InvalidateRequest() override
	{
		if (OnStartSessionCompleted.IsBound())
		{
			OnStartSessionCompleted.RemoveAll(this);
			OnStartSessionCompleted.Clear();
		}
		
		Super::InvalidateRequest();
	}
	//~ End UEnhancedOnlineRequestBase Interface
	
	FOnStartSessionComplete OnStartSessionCompleted;

protected:
	friend UEnhancedOnlineSessionsSubsystem;

	/** Online session pointer */
	IOnlineSessionPtr Sessions;
};

/**
 * Request class used to create an online lobby
 */
UCLASS()
class UEnhancedOnlineRequest_CreateLobby : public UEnhancedOnlineRequest_Session
{
	GENERATED_BODY()

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const override
	{
		return MaxPlayerCount <= 0 ? MaxNumConnectionsLobby : MaxPlayerCount;
	}

	/** Returns the full map name that will be loaded when the session is created */
	virtual FString GetMapName() const override
	{
		return GetWorld()->GetName();
	}

	/** Returns the travel URL of the current world */
	virtual FURL GetTravelURL() const override
	{
		FURL TravelURL = Super::GetTravelURL();
		TravelURL.Map = GetMapName();

		return TravelURL;
	}
};

/**
 * Delegate for when a user logs in
 * @param LocalUserIndex	The index of the local user who logged in
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUserLoginCompleted, int32 /* Local User Index */);

/**
 * Request class used to log in a user
 */
UCLASS()
class UEnhancedOnlineRequest_LoginUser : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	/** The type of authentication to use */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	EEnhancedLoginAuthType AuthType;

	/** The authentication token to use */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString AuthToken;

	/** The user id to login */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString UserId;

	/** Native delegate for when the user logs in */
	FOnUserLoginCompleted OnUserLoginCompleted;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Identity = OnlineSub->GetIdentityInterface();
		check(Identity);
	}

	virtual void InvalidateRequest() override
	{
		Super::InvalidateRequest();

		if (OnUserLoginCompleted.IsBound())
		{
			OnUserLoginCompleted.RemoveAll(this);
			OnUserLoginCompleted.Clear();
		}
	}

protected:
	friend UEnhancedOnlineSessionsSubsystem;
	IOnlineIdentityPtr Identity;
};

/**
 * Request class used to log out a user
 */
UCLASS()
class UEnhancedOnlineRequest_LogoutUser : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	/** Native delegate for when the user logs in */
	FOnUserLoginCompleted OnUserLogoutCompleted;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Identity = OnlineSub->GetIdentityInterface();
		check(Identity);
	}

	virtual void InvalidateRequest() override
	{
		Super::InvalidateRequest();

		if (OnUserLogoutCompleted.IsBound())
		{
			OnUserLogoutCompleted.RemoveAll(this);
			OnUserLogoutCompleted.Clear();
		}
	}

protected:
	friend UEnhancedOnlineSessionsSubsystem;
	IOnlineIdentityPtr Identity;
};

/**
 * A search result object that represents a session found online
 */
UCLASS(BlueprintType)
class UEnhancedSessionSearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** Pings the session to get the current ping in milliseconds */
	int32 GetPingInMs() const
	{
		return StoredSearchResult.PingInMs;
	}

	/** Returns the maximum number of players that can join the session */
	int32 GetMaxPlayers() const
	{
		return StoredSearchResult.Session.SessionSettings.NumPublicConnections;
	}

	/** Returns the number of players currently in the session */
	int32 GetCurrentPlayers() const
	{
		return StoredSearchResult.Session.SessionSettings.NumPublicConnections - StoredSearchResult.Session.NumOpenPublicConnections;
	}

	/** Returns the session name */
	FString GetSessionFriendlyName() const
	{
		if (StoredSearchResult.Session.SessionSettings.Settings.Array().Num() > 0)
		{
			for (auto& Setting : StoredSearchResult.Session.SessionSettings.Settings)
			{
				if (Setting.Key == SETTING_FRIENDLYNAME)
				{
					return Setting.Value.Data.ToString();
				}
			}
		}

		if (StoredSearchResult.Session.OwningUserId.IsValid())
		{
			return StoredSearchResult.Session.OwningUserName;
		}

		return TEXT("Unknown");
	}

public:
	/** The search result which uniquely identifies the session */
	FOnlineSessionSearchResult StoredSearchResult;
};

/**
 * Delegate for when a find online sessions request is completed
 * @param SearchResults	The search results found online
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindOnlineSessionsCompleted, const TArray<UEnhancedSessionSearchResult*> /* Search Results */);

/**
 * Request class used to find online sessions
 */
UCLASS()
class UEnhancedOnlineRequest_FindSessions : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	/** Specifies the online mode of the session */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	EEnhancedSessionOnlineMode OnlineMode;

	/** Whether to search for player-hosted lobbies */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	bool bFindLobbies;

	/** Maximum number of search results to return */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	int32 MaxSearchResults;

	/** A keyword that will be used to search and filter the sessions */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	FString SearchKeyword;

	/** List of all the search results found online, will be valid after the request is completed */
	UPROPERTY(BlueprintReadOnly, Category = "Online|Request")
	TArray<TObjectPtr<UEnhancedSessionSearchResult>> SearchResults;

	/** Native delegate for when the request is completed */
	FOnFindOnlineSessionsCompleted OnFindOnlineSessionsCompleted;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

	virtual void InvalidateRequest() override
	{
		Super::InvalidateRequest();

		if (OnFindOnlineSessionsCompleted.IsBound())
		{
			OnFindOnlineSessionsCompleted.RemoveAll(this);
			OnFindOnlineSessionsCompleted.Clear();
		}
	}

private:
	friend UEnhancedOnlineSessionsSubsystem;
	IOnlineSessionPtr Sessions;
};

/**
 * Request class used to join an online session
 */
UCLASS()
class UEnhancedOnlineRequest_JoinSession : public UEnhancedOnlineRequestBase
{
	GENERATED_BODY()

public:
	/** The session to join */
	UPROPERTY(BlueprintReadWrite, Category = "Online|Request")
	TObjectPtr<UEnhancedSessionSearchResult> SessionToJoin;

public:
	virtual void ConstructRequest() override
	{
		Super::ConstructRequest();

		Sessions = OnlineSub->GetSessionInterface();
		check(Sessions);
	}

private:
	friend UEnhancedOnlineSessionsSubsystem;
	IOnlineSessionPtr Sessions;
};

/**
 * Helper class for managing online search settings
 * Manages garbage collection
 */
class FEnhancedOnlineSearchSettingsBase : public FGCObject
{
public:
	FEnhancedOnlineSearchSettingsBase(UEnhancedOnlineRequest_FindSessions* InRequest)
	{
		Request = InRequest;
	}

	virtual ~FEnhancedOnlineSearchSettingsBase() {}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Request);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FEnhancedOnlineSearchSettingsBase");
		return NameString;
	}

public:
	TObjectPtr<UEnhancedOnlineRequest_FindSessions> Request = nullptr;
};

/**
 * Helper class for managing online search settings
 */
class FEnhancedOnlineSearchSettings : public FOnlineSessionSearch, public FEnhancedOnlineSearchSettingsBase
{
public:
	FEnhancedOnlineSearchSettings(UEnhancedOnlineRequest_FindSessions* InRequest)
		: FEnhancedOnlineSearchSettingsBase(InRequest)
	{
		bIsLanQuery = (InRequest->OnlineMode == EEnhancedSessionOnlineMode::LAN);
		MaxSearchResults = InRequest->MaxSearchResults <= 0 ? UINT_MAX : InRequest->MaxSearchResults;
		PingBucketSize = 100;

		if (InRequest->bFindLobbies)
		{
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		}

		if (!InRequest->SearchKeyword.IsEmpty())
		{
			QuerySettings.Set(SEARCH_KEYWORDS, InRequest->SearchKeyword, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FEnhancedOnlineSearchSettings() {}
};