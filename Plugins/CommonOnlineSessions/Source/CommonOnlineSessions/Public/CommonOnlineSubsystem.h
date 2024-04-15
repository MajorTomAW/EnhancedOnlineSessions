// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonOnlineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonOnlineSubsystem.generated.h"

class FCommonOnlineSessionSearchSettings;

/************************************************************************************
 * Common Online Session Requests													*
 ************************************************************************************/

/**
 * A request object that can be used to interact with online services.
 */
UCLASS(BlueprintType, Abstract)
class COMMONONLINESESSIONS_API UCommonOnlineRequest : public UObject
{
	GENERATED_BODY()

public:
	void InvalidateRequest()
	{
		MarkAsGarbage();
		UnbindDelegates();
	}

	virtual void UnbindDelegates()
	{
		OnFailed.RemoveAll(this);
		OnFailed.Clear();
	}

	FOnRequestFailedWithLog OnFailed;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLoginUserSuccess, int32 LocalUserIndex, const FUniqueMessageId& UserId);
/**
 * A request object that stores the credentials needed to log in to an online service.
 */
UCLASS(DisplayName = "Login User Request")
class UCommonOnline_LoginUserRequest : public UCommonOnlineRequest
{
	GENERATED_BODY()

public:
	/** The user id that will be used to log in */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Login")
	FString UserId;

	/** The user token, associated with the user id */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Login")
	FString UserToken;

	/** The type of authentication to use */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Login")
	EAuthType AuthType;

	FOnLoginUserSuccess OnLoginUserSuccess;
};

/**
 * A request object that stores the session settings needed to create a new session.
 */
UCLASS(DisplayName = "Create Session Request")
class UCommonOnline_CreateSessionRequest : public UCommonOnlineRequest
{
	GENERATED_BODY()

public:
	/** Indicates if the session is a full online session or a different type */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	ECommonSessionOnlineMode OnlineMode;

	/** The maximum number of players that can join the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 MaxPlayerCount;

	/** True if this request should create a player-hosted lobby if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	uint8 bUseLobbiesIfAvailable:1;

	/** True if this request should use voice chat if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	uint8 bUseVoiceChatIfAvailable:1;

	/** Friendly name for the session, used for display purposes */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString SessionFriendlyName;

	/** The search keyword that will be used to find the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString SearchKeyword;

	/** Friendly name for the game mode, used for display purposes */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString GameModeFriendlyName;

	/** The map to load for the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session", meta = (AllowedTypes = "World"))
	FPrimaryAssetId MapID;

	/** Custom user data to pass to the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FStoredSessionSettings StoredSettings;

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const;

	/** Returns the full map name to load for the session */
	virtual FString GetMapName() const;

	/** Constructs the full URL to travel to the session */
	virtual FString ConstructTravelURL() const;

	/** Validates the request and logs any errors */
	virtual bool ValidateAndLogErrors(FText& OutError) const;
};

/**
 * Helper class to store session settings
 */
class FCommonOnline_OnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FCommonOnline_OnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4)
	{
		NumPublicConnections = MaxNumPlayers;
		if (NumPublicConnections < 0)
		{
			NumPublicConnections = 0;
		}
		NumPrivateConnections = 0;
		bIsLANMatch = bIsLAN;
		bShouldAdvertise = true;
		bAllowJoinInProgress = true;
		bAllowInvites = true;
		bUsesPresence = bIsPresence;
		bAllowJoinViaPresence = true;
		bAllowJoinViaPresenceFriendsOnly = false;
	}

	virtual ~FCommonOnline_OnlineSessionSettings() {}
};

UCLASS(BlueprintType)
class UCommonOnlineSearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** Ping to the search result */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session")
	int32 GetPingInMs() const;

	/** The number of players currently in the session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session")
	int32 GetNumCurrentPlayers() const;

	/** The maximum number of players that can join the session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session")
	int32 GetNumMaxPlayers() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Session")
	FString GetSessionNameSafe() const;

	FOnlineSessionSearchResult Result;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsSuccess, const TArray<UCommonOnlineSearchResult*>& SearchResults);
/**
 * A request object that stores the settings needed to find a session.
 */
UCLASS(DisplayName = "Find Sessions Request")
class UCommonOnline_FindSessionsRequest : public UCommonOnlineRequest
{
	GENERATED_BODY()

public:
	/** Indicates if the session is a full online session or a different type */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	ECommonSessionOnlineMode OnlineMode;

	/** The maximum amount of search results to return, -1 means no limit */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 MaxSearchResults;

	/** True if this request should create a player-hosted lobby if available */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bSearchLobbies;

	/** List of all found search results */
	UPROPERTY(BlueprintReadOnly, Category = "Session")
	TArray<TObjectPtr<UCommonOnlineSearchResult>> SearchResults;

	/** Called when the search is successful */
	FOnFindSessionsSuccess OnFindSessionsSuccess;
};

/************************************************************************************
 * UCommonOnlineSubsystem															*
 ************************************************************************************/

/**
 * Game subsystem that handles requests to various online services.
 * Including login, lobbies, etc.
 */
UCLASS()
class COMMONONLINESESSIONS_API UCommonOnlineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonOnlineSubsystem();

	//~ Begin UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UGameInstanceSubsystem Interface

	/** Logs in a user with the specified credentials, this is used to authenticate with online services */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Identity")
	virtual void LoginOnlineUser(APlayerController* PlayerToLogin, UCommonOnline_LoginUserRequest* LoginRequest);

	/** Creates a new online session with the specified settings */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void CreateOnlineSession(APlayerController* HostingPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest);

	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void FindOnlineSessions(APlayerController* PlayerSearching, UCommonOnline_FindSessionsRequest* FindSessionsRequest);

protected:
	virtual void BindOnlineDelegates();
	virtual void LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LoginUserRequest* LoginRequest);
	virtual void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest);
	virtual void FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FCommonOnlineSessionSearchSettings>& InSearchSettings);

	virtual void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;
	
	/** Most recent result info of the last online action */
	FOnlineResultInfo LastOnlineResult;

	/** Stored session settings for creating a session */
	TSharedPtr<FCommonOnline_OnlineSessionSettings> SessionSettings;

	/** Stored search settings for finding a session */
	TSharedPtr<FCommonOnlineSessionSearchSettings> SearchSettings;
};
