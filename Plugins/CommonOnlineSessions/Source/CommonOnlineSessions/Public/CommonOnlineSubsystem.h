// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonOnlineTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonOnlineSubsystem.generated.h"

class FCommonOnlineSessionSearchSettings;
class UCommonOnlineSearchResult;

DECLARE_MULTICAST_DELEGATE_OneParam(FCommonOnlineSessionOnPreClientTravel, FString& /* Travel URL */);

/**
 * Delegate for when a user has accepted a session invite or requested to join a session
 * @param Platform User Id		The platform user id of the user that requested the session, note that the user may not be logged in yet.
 * @param Requested Session		The session that the user requested to join, this may be null if the session no longer exists.
 * @param Result Info			The result info of the operation
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCommonOnlineSessionOnUserRequestedSession, const FPlatformUserId& /* Platform User Id */, UCommonOnlineSearchResult* /* Requested Session */, const FOnlineResultInfo& /* Result Info */);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlueprintCommonOnlineSessionOnUserRequestedSession, const FPlatformUserId&, PlatformUserId, UCommonOnlineSearchResult*, RequestedSession, const FOnlineResultInfo&, ResultInfo);

/**
 * Delegate for when a user has received a session invite
 * @param Received User Id		The user id of the user that received the invite
 * @param Sending User Id		The user id of the user that sent the invite
 * @param Invite Result			The search result of the invite
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FCommonOnlineSessionOnUserReceivedSessionInvite, const FUniqueNetId& /* Received User Id */, const FUniqueNetId& /* Sending User Id */, UCommonOnlineSearchResult* /* Invite Result */, const FOnlineResultInfo& /* Result Info */);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBlueprintCommonOnlineSessionOnUserReceivedSessionInvite, const FUniqueNetIdRepl&, ReceivedUserId, const FUniqueNetIdRepl&, SendingUserId, UCommonOnlineSearchResult*, InviteResult, const FOnlineResultInfo&, ResultInfo);

/************************************************************************************
 * Common Online Session Requests													*
 ************************************************************************************/
#pragma region requests
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
 * An empty request object needed to log out of an online service.
 */
UCLASS(DisplayName = "Logout User Request")
class UCommonOnline_LogoutUserRequest : public UCommonOnlineRequest
{
	GENERATED_BODY()

public:
	FOnEmptyIndexRequestSuccess OnLogoutUserSuccess;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGetFriendsListComplete, const TArray<FCommonOnlineFriendInfo>& FriendsList);
/**
 * A request object that stores the settings needed to get the friends list of a user.
 */
UCLASS(DisplayName = "Get Friends List Request")
class UCommonOnline_GetFriendsListRequest : public UCommonOnlineRequest
{
	GENERATED_BODY()

public:
	/** Online stat filter, used to only get friends that have a specific online state */
	UPROPERTY(BlueprintReadWrite, Category = "Friends")
	ECommonFriendOnlineSateFilter OnlineFilter;

	FOnGetFriendsListComplete OnGetFriendsListComplete;
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

	/** Whether to use server travel on success */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseServerTravelOnSuccess = false;

	/** Custom user data to pass to the session */
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FStoredSessionSettings StoredSettings;

public:
	/** Returns the maximum number of players that can join the session */
	virtual int32 GetMaxPlayers() const;

	/** Returns the full map name to load for the session */
	virtual FString GetMapName() const;

	/** Constructs the full URL to travel to the session */
	virtual FString ConstructTravelURL(bool bWithServerTravel = true) const;

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
#pragma endregion requests

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
	UCommonOnlineSubsystem() {}

	//~ Begin UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UGameInstanceSubsystem Interface

	/** Logs in a user with the specified credentials, this is used to authenticate with online services */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Identity")
	virtual void LoginOnlineUser(APlayerController* PlayerToLogin, UCommonOnline_LoginUserRequest* LoginRequest);

	/** Logs out the specified user */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Identity")
	virtual void LogoutOnlineUser(APlayerController* PlayerToLogout, UCommonOnline_LogoutUserRequest* LogoutRequest);

	/** Tries to get the user friends list of the online service */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Friends")
	virtual void GetOnlineFriendsList(APlayerController* PlayerRequesting, UCommonOnline_GetFriendsListRequest* FriendsRequest);




	
	
	/** Creates a new online session with the specified settings */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void CreateOnlineSession(APlayerController* HostingPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest);

	/** Finds online sessions that match the specified search settings */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void FindOnlineSessions(APlayerController* PlayerSearching, UCommonOnline_FindSessionsRequest* FindSessionsRequest);

	/** Joins the specified online session */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void JoinOnlineSession(APlayerController* JoiningPlayer, UCommonOnlineSearchResult* Session);

	/** Called to clean up any active sessions */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Sessions")
	virtual void CleanupOnlineSessions();

protected:
	virtual void BindOnlineDelegates();

	/** Online Sessions */
	virtual void BindOnlineSessionDelegates(IOnlineSubsystem* OnlineSub);
	virtual void RemoveOnlineSessionDelegates(IOnlineSubsystem* OnlineSub);
	virtual void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest);
	virtual void FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FCommonOnlineSessionSearchSettings>& InSearchSettings);
	virtual void JoinOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonOnlineSearchResult* Session);
	virtual void TravelToSessionInternal(const FName SessionName);
	virtual void CleanupOnlineSessionsInternal();

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionsComplete(bool bWasSuccessful);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& InviteResult);
	virtual void OnSessionInviteReceived(const FUniqueNetId& ReceivedUserId, const FUniqueNetId& SendingUserId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);

	void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonOnlineSearchResult* RequestedSession, const FOnlineResultInfo& ResultInfo);
	void NotifyUserReceivedSessionInvite(const FUniqueNetId& ReceivedUserId, const FUniqueNetId& SendingUserId, UCommonOnlineSearchResult* InviteResult, const FOnlineResultInfo& ResultInfo);

	/** Online Identity */
	virtual void BindOnlineIdentityDelegates(IOnlineSubsystem* OnlineSub);
	virtual void RemoveOnlineIdentityDelegates(IOnlineSubsystem* OnlineSub);
	virtual void LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LoginUserRequest* LoginRequest);
	virtual void LogoutOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LogoutUserRequest* LogoutRequest);
	virtual void GetOnlineFriendsListInternal(ULocalPlayer* LocalPlayer, UCommonOnline_GetFriendsListRequest* FriendsRequest);

	virtual void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	virtual void OnLoginStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId);
	virtual void OnGetFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString);
	
	void NotifyUserLoginComplete(int32 LocalUserNum, const FUniqueNetId& UserId, const FOnlineResultInfo& ResultInfo);
	void NotifyUserLoginStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId);

protected:
	/** Pending friends list request which will be destroyed when finished */
	UPROPERTY()
	UCommonOnline_GetFriendsListRequest* PendingFriendsListRequest;
	
	/** Native delegate for modifying the travel URL before traveling to a session */
	FCommonOnlineSessionOnPreClientTravel OnPreClientTravelEvent;
	
	/** Native delegate for when a user has accepted a session invite, or requested to join a session */
	FCommonOnlineSessionOnUserRequestedSession OnUserRequestedSessionEvent;

	/** Native broadcast for when a user has accepted a session invite, or requested to join a session */
	UPROPERTY(BlueprintAssignable, Category = "Online|Common Sessions|Events", meta = (DisplayName = "On User Requested Session"))
	FBlueprintCommonOnlineSessionOnUserRequestedSession K2_OnUserRequestedSessionEvent;

	/** Native delegate for when a user has received a session invite */
	FCommonOnlineSessionOnUserReceivedSessionInvite OnUserReceivedSessionInviteEvent;

	/** Native broadcast for when a user has received a session invite */
	UPROPERTY(BlueprintAssignable, Category = "Online|Common Sessions|Events", meta = (DisplayName = "On User Received Session Invite"))
	FBlueprintCommonOnlineSessionOnUserReceivedSessionInvite K2_OnUserReceivedSessionInviteEvent;
	
	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;

	/** Whether to destroy the pending session after the next operation */
	bool bWantsToDestroyPendingSession = false;
	
	/** Most recent result info of the last online action */
	FOnlineResultInfo LastOnlineResult;

	/** Stored session settings for creating a session */
	TSharedPtr<FCommonOnline_OnlineSessionSettings> SessionSettings;

	/** Stored search settings for finding a session */
	TSharedPtr<FCommonOnlineSessionSearchSettings> SearchSettings;
};
