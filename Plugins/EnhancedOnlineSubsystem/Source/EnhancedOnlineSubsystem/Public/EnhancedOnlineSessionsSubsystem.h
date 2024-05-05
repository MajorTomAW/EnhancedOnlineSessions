// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EnhancedOnlineSessionsSubsystem.generated.h"

class UEnhancedOnlineRequest_StartSession;
class UEnhancedOnlineRequest_LogoutUser;
class UEnhancedOnlineRequest_JoinSession;
class UEnhancedSessionSearchResult;
class FEnhancedOnlineSearchSettings;
class UEnhancedOnlineRequest_FindSessions;
class UEnhancedOnlineRequest_LoginUser;
class FEnhancedOnlineSessionSettings;
class UEnhancedOnlineRequest_CreateLobby;
class UEnhancedOnlineRequest_CreateSession;
class UEnhancedOnlineRequest_Session;
class FOnlineSessionSearch;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginYes);

/**
 * Subsystem for managing online sessions and communication with the online service.
 */
UCLASS(DisplayName = "Enhanced Online Subsystem", meta = (DisplayName = "Enhanced Online Subsystem"))
class ENHANCEDONLINESUBSYSTEM_API UEnhancedOnlineSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEnhancedOnlineSessionsSubsystem() {}

	//~ Begin UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UGameInstanceSubsystem Interface

#pragma region online_identity
	/**
	 * Logs in the online user.
	 * @param Request	The request object that contains the login settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Identity")
	virtual void LoginOnlineUser(UEnhancedOnlineRequest_LoginUser* Request);

	/**
	 * Logs out the online user.
	 * @param Request	The request object that contains the logout settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Identity")
	virtual void LogoutOnlineUser(UEnhancedOnlineRequest_LogoutUser* Request);
#pragma endregion
	
#pragma region online_sessions
	/**
	 * Hosts an online session.
	 * @param Request	The request object that contains the session settings, could be a session or a P2P Lobby.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void HostOnlineSession(UEnhancedOnlineRequest_Session* Request);

	/**
	 * Starts the current online session
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void StartOnlineSession(UEnhancedOnlineRequest_StartSession* Request);

	/**
	 * Finds online sessions.
	 * @param Request	The request object that contains the search settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void FindOnlineSessions(UEnhancedOnlineRequest_FindSessions* Request);

	/**
	 * Joins an online session.
	 * @param Request	The search result of the session to join.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void JoinOnlineSession(UEnhancedOnlineRequest_JoinSession* Request);
#pragma endregion

protected:
	/** Online Sessions */
	virtual void HostOnlineSessionInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateSession* Request);
	virtual void HostOnlineLobbyInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateLobby* Request);
	virtual void FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FEnhancedOnlineSearchSettings>& InSearchSettings);

	FDelegateHandle HostLobbyDelegateHandle;
	FDelegateHandle HostSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle StartSessionDelegateHandle;

	virtual void HandleHostOnlineLobbyComplete(FName SessionName, bool bWasSuccessful);
	virtual void HandleHostOnlineSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void HandleStartOnlineSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void HandleFindOnlineSessionsComplete(bool bWasSuccessful);
	virtual void HandleJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** Online Identity */
	virtual void LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LoginUser* Request);
	virtual void LogoutOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LogoutUser* Request);

	FDelegateHandle LoginDelegateHandle;
	FDelegateHandle LogoutDelegateHandle;

	virtual void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	virtual void HandleLogoutComplete(int32 LocalUserNum, bool bWasSuccessful);


private:
	/** The URL to travel to after the session is created */
	FURL PendingTravelURL;

	/** The URL to travel to after the client joins the session */
	FString PendingClientTravelURL;

	/** The request object for the pending session */
	UPROPERTY()
	TObjectPtr<UEnhancedOnlineRequest_Session> PendingSessionRequest;

	/** The request object for the pending login */
	UPROPERTY()
	TObjectPtr<UEnhancedOnlineRequest_LoginUser> PendingLoginRequest;

	/** The request object for the pending logout */
	UPROPERTY()
	TObjectPtr<UEnhancedOnlineRequest_LogoutUser> PendingLogoutRequest;

	/** The request object for the pending join session */
	UPROPERTY()
	TObjectPtr<UEnhancedOnlineRequest_StartSession> PendingStartSessionRequest;

	/** Session settings for the pending session */
	TSharedPtr<FEnhancedOnlineSessionSettings> SessionSettings;

	/** Settings for the current search */
	TSharedPtr<FEnhancedOnlineSearchSettings> SearchSettings;
};
