// Copyright © 2024 MajorT. All rights reserved.

#include "CommonOnlineSubsystem.h"

#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AssetRegistry/AssetData.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"

void UCommonOnlineSubsystem::BindOnlineSessionDelegates(IOnlineSubsystem* OnlineSub)
{
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());
	
	// Bind to the session delegates
	Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));
	Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	Sessions->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted));
	Sessions->AddOnSessionInviteReceivedDelegate_Handle(FOnSessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived));
}

void UCommonOnlineSubsystem::RemoveOnlineSessionDelegates(IOnlineSubsystem* OnlineSub)
{
	const IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (Sessions)
	{
		Sessions->ClearOnSessionFailureDelegates(this);
	}
}


#pragma region session_invites
void UCommonOnlineSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& InviteResult)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(ControllerId);
	UCommonOnlineSearchResult* RequestedSession = nullptr;
	FOnlineResultInfo ResultInfo;

	if (bWasSuccessful)
	{
		RequestedSession = NewObject<UCommonOnlineSearchResult>(this);
		RequestedSession->Result = InviteResult;
	}
	else
	{
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("SessionInviteFailed");
		ResultInfo.ErrorMessage = TEXT("Failed to accept session invite");
	}

	NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
}

void UCommonOnlineSubsystem::NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonOnlineSearchResult* RequestedSession, const FOnlineResultInfo& ResultInfo)
{
	OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, ResultInfo);
	K2_OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, ResultInfo);
}

void UCommonOnlineSubsystem::OnSessionInviteReceived(const FUniqueNetId& ReceivedUserId, const FUniqueNetId& SendingUserId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	UCommonOnlineSearchResult* RequestedSession = nullptr;
	FOnlineResultInfo ResultInfo;

	if (InviteResult.IsValid())
	{
		RequestedSession = NewObject<UCommonOnlineSearchResult>(this);
		RequestedSession->Result = InviteResult;
	}
	else
	{
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("SessionInviteFailed");
		ResultInfo.ErrorMessage = TEXT("Failed to receive session invite");
	}

	NotifyUserReceivedSessionInvite(ReceivedUserId, SendingUserId, RequestedSession, ResultInfo);
}

void UCommonOnlineSubsystem::NotifyUserReceivedSessionInvite(const FUniqueNetId& ReceivedUserId, const FUniqueNetId& SendingUserId, UCommonOnlineSearchResult* InviteResult, const FOnlineResultInfo& ResultInfo)
{
	OnUserReceivedSessionInviteEvent.Broadcast(ReceivedUserId, SendingUserId, InviteResult, ResultInfo);

	FUniqueNetIdRepl ReceivedUserIdRepl(ReceivedUserId);
	FUniqueNetIdRepl SendingUserIdRepl(SendingUserId);
	K2_OnUserReceivedSessionInviteEvent.Broadcast(ReceivedUserIdRepl, SendingUserIdRepl, InviteResult, ResultInfo);
}
#pragma endregion

#pragma region create_online_session
void UCommonOnlineSubsystem::CreateOnlineSession(APlayerController* HostingPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest)
{
	// Make sure the request is valid
	if (CreateSessionRequest == nullptr)
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Create Online Session was called with a bad request"));
		return;
	}

	// Get the local player and return if it's invalid
	ULocalPlayer* LocalPlayer = (HostingPlayer != nullptr) ? HostingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Unable to find local player"));
		return;
	}

	// Validate the request
	FText OutError;
	if (!CreateSessionRequest->ValidateAndLogErrors(OutError))
	{
		CreateSessionRequest->OnFailed.Broadcast(OutError.ToString());
		return;
	}

	// If we're hosting an offline game, just travel to the map
	if (CreateSessionRequest->OnlineMode == ECommonSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			CreateSessionRequest->OnFailed.Broadcast(TEXT("Unable to host offline game as client."));
		}
		else
		{
			GetWorld()->ServerTravel(CreateSessionRequest->ConstructTravelURL());
		}
	}
	else
	{
		CreateOnlineSessionInternal(LocalPlayer, CreateSessionRequest);
	}
}

void UCommonOnlineSubsystem::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest)
{
	LastOnlineResult = FOnlineResultInfo();

	const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());
	
	PendingTravelURL = CreateSessionRequest->ConstructTravelURL(CreateSessionRequest->bUseServerTravelOnSuccess);

	if (CreateSessionRequest->bUseServerTravelOnSuccess && OnlineSub->GetSubsystemName() == "STEAM")
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Seems like u want to do server travel when hosting a session, but Steam doesn't like that. you may find a away around that if you know what you are doing, but for now, you should disable bUseServerTravelOnSuccess."));
		UE_LOG(LogOnlineSession, Error, TEXT("Steam doesn't like server travel when hosting a session, you may find a way around that if you know what you are doing, but for now, you should disable bUseServerTravelOnSuccess."));
	}

	// Initialize the session
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = CreateSessionRequest->GetMaxPlayers();
	const bool bIsPresence = CreateSessionRequest->bUseLobbiesIfAvailable;

	FUniqueNetIdPtr UserId;
	if (LocalPlayer)
	{
		UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	}
	else
	{
		UserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);
	}

	// Create the session settings
	if (ensure(UserId.IsValid()))
	{
		SessionSettings = MakeShareable(new FCommonOnline_OnlineSessionSettings(CreateSessionRequest->OnlineMode == ECommonSessionOnlineMode::LAN, bIsPresence, MaxPlayers));
		SessionSettings->bUseLobbiesIfAvailable = CreateSessionRequest->bUseLobbiesIfAvailable;
		SessionSettings->bUseLobbiesVoiceChatIfAvailable = CreateSessionRequest->bUseVoiceChatIfAvailable;
		SessionSettings->Set(SETTING_GAMEMODE, CreateSessionRequest->GameModeFriendlyName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MAPNAME, CreateSessionRequest->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SEARCH_KEYWORDS, CreateSessionRequest->SearchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

		FSessionSettings& UserSettings = SessionSettings->MemberSettings.Add(UserId.ToSharedRef(), FSessionSettings());
		UserSettings.Add(SETTING_GAMEMODE, FOnlineSessionSetting(FString("GameSession"), EOnlineDataAdvertisementType::ViaOnlineService));

		// Try to get a friendly name for the session
		FString FriendlyName = CreateSessionRequest->SessionFriendlyName;
		if (FriendlyName.IsEmpty())
		{
			FriendlyName = FText::FromName(NAME_GameSession).ToString();
		}

		// Add the friendly name
		SessionSettings->Settings.Add("FRIENDLYNAME", FOnlineSessionSetting(FriendlyName, EOnlineDataAdvertisementType::ViaOnlineService));

		// Add the additional stored settings
		for (const auto& StoredSetting : CreateSessionRequest->StoredSettings.StoredSettings)
		{
			SessionSettings->Settings.Add(StoredSetting.Key, FOnlineSessionSetting(StoredSetting.Data.ToString(), EOnlineDataAdvertisementType::ViaOnlineService));
		}
		
		Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
	}
	else
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Failed to ge a valid user id"));
	}
}

void UCommonOnlineSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// If the session was created successfully, travel to the map
	if (bWasSuccessful)
	{
		LastOnlineResult = FOnlineResultInfo();
		LastOnlineResult.bWasSuccessful = true;

		GetWorld()->ServerTravel(PendingTravelURL);
	}
	else
	{
		if (LastOnlineResult.bWasSuccessful || LastOnlineResult.ErrorMessage.IsEmpty())
		{
			LastOnlineResult.bWasSuccessful = false;
		}

		UE_LOG(LogOnlineSession, Error, TEXT("FinishSessionCreation(%s): %s"), *LastOnlineResult.ErrorId, *LastOnlineResult.ErrorMessage);
	}
}
#pragma endregion

#pragma region find_online_sessions

/** Helper class for the search settings, manages the garbage collection of the search request */
class FCommonOnlineSearchSettingsBase : public FGCObject
{
public:
	FCommonOnlineSearchSettingsBase(UCommonOnline_FindSessionsRequest* InSearchRequest)
	{
		SearchRequest = InSearchRequest;
	}

	virtual ~FCommonOnlineSearchSettingsBase() {}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SearchRequest);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FCommonOnlineSearchSettings");
		return NameString;
	}

public:
	TObjectPtr<UCommonOnline_FindSessionsRequest> SearchRequest = nullptr;
};

/** Helper class for the search settings */
class FCommonOnlineSessionSearchSettings : public FOnlineSessionSearch, public FCommonOnlineSearchSettingsBase
{
public:
	FCommonOnlineSessionSearchSettings(UCommonOnline_FindSessionsRequest* InSearchRequest)
		: FCommonOnlineSearchSettingsBase(InSearchRequest)
	{
		bIsLanQuery = (InSearchRequest->OnlineMode == ECommonSessionOnlineMode::LAN);
		MaxSearchResults = InSearchRequest->MaxSearchResults;
		PingBucketSize = 100;

		if (InSearchRequest->bSearchLobbies)
		{
			QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FCommonOnlineSessionSearchSettings() {}
};

void UCommonOnlineSubsystem::FindOnlineSessions(APlayerController* PlayerSearching, UCommonOnline_FindSessionsRequest* FindSessionsRequest)
{
	// Make sure the request is valid
	if (FindSessionsRequest == nullptr)
	{
		FindSessionsRequest->OnFailed.Broadcast(TEXT("Find Online Sessions was called with a bad request"));
		return;
	}

	// Get the local player and return if it's invalid
	ULocalPlayer* LocalPlayer = (PlayerSearching != nullptr) ? PlayerSearching->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		FindSessionsRequest->OnFailed.Broadcast(TEXT("Unable to find local player"));
		return;
	}

	FindOnlineSessionsInternal(LocalPlayer, MakeShared<FCommonOnlineSessionSearchSettings>(FindSessionsRequest));
}

void UCommonOnlineSubsystem::FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FCommonOnlineSessionSearchSettings>& InSearchSettings)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	SearchSettings = InSearchSettings;

	if (!Sessions->FindSessions(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), StaticCastSharedRef<FCommonOnlineSessionSearchSettings>(SearchSettings.ToSharedRef())))
	{
		InSearchSettings->SearchRequest->OnFailed.Broadcast(TEXT("Failed to find sessions, see log for details"));
	}
}

void UCommonOnlineSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnFindSessionsComplete(bWasSuccessful: %s)"), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!SearchSettings.IsValid())
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnFindSessionsComplete: Search settings are invalid"));
		return;
	}

	if (SearchSettings->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		SearchSettings->SearchRequest->OnFailed.Broadcast(TEXT("Unable to find sessions, previous search request is still in progress"));
		return;
	}

	if (!ensure(SearchSettings->SearchRequest))
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnFindSessionsComplete: Search request is invalid"));
		return;
	}

	// Store the search results into the search request if the search was successful
	if (bWasSuccessful)
	{
		for (const FOnlineSessionSearchResult& Result : SearchSettings->SearchResults)
		{
			UCommonOnlineSearchResult* Entry = NewObject<UCommonOnlineSearchResult>(SearchSettings->SearchRequest);
			Entry->Result = Result;
			SearchSettings->SearchRequest->SearchResults.Add(Entry);

			FString OwningUserId = TEXT("Unknown");
			if (Result.Session.OwningUserId.IsValid())
			{
				OwningUserId = Result.Session.OwningUserId->ToString();
			}

			UE_LOG(LogOnlineSession, Log, TEXT("\tFound session (UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
				*OwningUserId,
				*Result.Session.OwningUserName,
				Result.Session.NumOpenPrivateConnections,
				Result.Session.NumOpenPublicConnections,
				Result.PingInMs
				);
		}

		SearchSettings->SearchRequest->OnFindSessionsSuccess.Broadcast(SearchSettings->SearchRequest->SearchResults);
	}
	else
	{
		SearchSettings->SearchRequest->SearchResults.Empty();
		SearchSettings->SearchRequest->OnFailed.Broadcast(TEXT("Failed to find sessions, see log for details"));
	}

	SearchSettings.Reset();
}
#pragma endregion

#pragma region join_online_session
void UCommonOnlineSubsystem::JoinOnlineSession(APlayerController* JoiningPlayer, UCommonOnlineSearchResult* Session)
{
	// Make sure the session is valid
	if (Session == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Join Online Session was called with a bad session"));
		return;
	}

	// Get the local player and return if it's invalid
	ULocalPlayer* LocalPlayer = (JoiningPlayer != nullptr) ? JoiningPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Unable to find local player"));
		return;
	}

	JoinOnlineSessionInternal(LocalPlayer, Session);
}

void UCommonOnlineSubsystem::JoinOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonOnlineSearchResult* Session)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	Sessions->JoinSession(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, Session->Result);
}

void UCommonOnlineSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// If the session was joined successfully, travel to the map
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		TravelToSessionInternal(SessionName);
	}
	else
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			{
				ReturnReason = NSLOCTEXT("NetworkErrors", "SessionIsFull", "Game is full.");
				break;
			}
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			{
				ReturnReason = NSLOCTEXT("NetworkErrors", "SessionDoesNotExist", "Game session does not exist.");
				break;
			}
		default:
			{
				ReturnReason = NSLOCTEXT("NetworkErrors", "UnknownError", "Unknown error.");
				break;
			}
		}

		UE_LOG(LogOnlineSession, Error, TEXT("FinishJoinSession(%s) failed with Result: %s"), *SessionName.ToString(), *ReturnReason.ToString());
	}
}

void UCommonOnlineSubsystem::TravelToSessionInternal(const FName SessionName)
{
	// Get the player controller and return if it's invalid
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		FText ErrorText = NSLOCTEXT("NetworkErrors", "NoPlayerController", "No player controller found to travel to session.");
		UE_LOG(LogOnlineSession, Error, TEXT("%s"), *ErrorText.ToString());
		return;
	}

	FString TravelURL;

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	if (!Sessions->GetResolvedConnectString(SessionName, TravelURL))
	{
		FText ErrorText = NSLOCTEXT("NetworkErrors", "FailedToResolveConnectString", "Failed to resolve connect string for session.");
		UE_LOG(LogOnlineSession, Error, TEXT("%s"), *ErrorText.ToString());
		return;
	}

	// Client travel to the session URL
	OnPreClientTravelEvent.Broadcast(TravelURL);
	PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute);
}
#pragma endregion

#pragma region cleanup_online_sessions
void UCommonOnlineSubsystem::CleanupOnlineSessions()
{
	bWantsToDestroyPendingSession = true;
	SessionSettings.Reset();

	CleanupOnlineSessionsInternal();
}

void UCommonOnlineSubsystem::CleanupOnlineSessionsInternal()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	EOnlineSessionState::Type SessionState = Sessions->GetSessionState(NAME_GameSession);
	UE_LOG(LogOnlineSession, Log, TEXT("Session state is %s"), EOnlineSessionState::ToString(SessionState));

	if (SessionState == EOnlineSessionState::InProgress)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Ending session"))
		Sessions->EndSession(NAME_GameSession);
	}
	else if (SessionState == EOnlineSessionState::Ended || SessionState == EOnlineSessionState::Pending)
	{
		Sessions->DestroySession(NAME_GameSession);
	}
	else if (SessionState == EOnlineSessionState::Starting || SessionState == EOnlineSessionState::Creating)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Session is still starting or creating, waiting for it to finish"))
	}
	else
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Session is in an unknown state, not sure what to do"))
	}
}
#pragma endregion