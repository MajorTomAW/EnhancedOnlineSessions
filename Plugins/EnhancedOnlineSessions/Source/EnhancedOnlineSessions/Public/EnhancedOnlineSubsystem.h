// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedOnlineRequests.h"
#include "OnlineSessionSettings.h"
#include "EnhancedOnlineRequests.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EnhancedOnlineSubsystem.generated.h"

class FEnhancedOnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FEnhancedOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4)
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

	virtual ~FEnhancedOnlineSessionSettings() {}
};

/************************************************************************************
 * UEnhancedOnlineSubsystem                                                         *
 ************************************************************************************/

class UEnhancedOnlineRequest_LoginUser;
/**
 * Game subsystem that handles requests to various online services.
 * Including login, lobbies and more.
 */
UCLASS()
class ENHANCEDONLINESESSIONS_API UEnhancedOnlineSubsystem : public UGameInstanceSubsystem
{
   GENERATED_BODY()

public:
	UEnhancedOnlineSubsystem() {}

	//~ Begin UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UGameInstanceSubsystem Interface

	void BindOnlineDelegates();
	void BindSessionDelegates(IOnlineSessionPtr Sessions);
	void BindIdentityDelegates(IOnlineIdentityPtr Identity);

#pragma region online_identity
	/** Logs in a local user with the given credentials, used to authenticate with the online service */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Identity")
	virtual void LoginOnlineUser(UEnhancedOnlineRequest_LoginUser* Request);
#pragma endregion

#pragma region online_sessions
	/** Creates a new session with the given settings */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void CreateOnlineSession(UEnhancedOnlineRequest_CreateSession* Request);

	/** Joins a session with the given search result */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void JoinOnlineSession(UEnhancedOnlineRequest_JoinSession* Request);

	/** Searches for sessions with the given search settings */
	UFUNCTION(BlueprintCallable, Category = "Online|EnhancedSessions|Sessions")
	virtual void FindOnlineSessions(UEnhancedOnlineRequest_SearchSessions* Request);
#pragma endregion

protected:
	/** Online Identity */
	virtual void LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LoginUser* Request);

	/** Online Sessions */
	virtual void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateSession* Request);
	virtual void JoinOnlineSessionInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_JoinSession* Request);
	virtual void TravelToSessionInternal(APlayerController* PlayerController, const FName SessionName);
	virtual void FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FEnhancedOnlineSearchSettings>& InSearchSettings);


	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;

	/** The pending join request that will be used after session operations are complete */
	UPROPERTY()
	TObjectPtr<UEnhancedOnlineRequest_JoinSession> PendingJoinRequest;

	/** Settings for the current host request */
	TSharedPtr<FEnhancedOnlineSessionSettings> SessionSettings;

	/** Settings for the current search */
	TSharedPtr<FEnhancedOnlineSearchSettings> SearchSettings;

	/** True if this is a dedicated server, which doesn't require a LocalPlayer to create a session */
	bool bIsDedicatedServer = false;
};
