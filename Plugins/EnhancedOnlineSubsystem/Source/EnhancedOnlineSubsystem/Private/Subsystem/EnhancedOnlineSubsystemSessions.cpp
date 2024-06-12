// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedOnlineSessionsSubsystem.h"

#include "EnhancedOnlineRequests.h"
#include "EnhancedOnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UEnhancedOnlineSessionsSubsystem::HostOnlineSession(UEnhancedOnlineRequest_Session* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Host Online Session was called with a bad request."));
		return;
	}

	if (IsValid(PendingSessionRequest))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("A session is already being hosted."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("A session is already being hosted."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Host Online Session was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Host Online Session was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Host Online Session was called with a bad local user index: %d."), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Host Online Session was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	if (Request->OnlineMode == EEnhancedSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			Request->OnRequestFailedDelegate.Broadcast(TEXT("Cannot host an offline session on a client."));
			return;
		}

		GetWorld()->ServerTravel(Request->GetTravelURL().ToString());
	}
	else
	{
		if (Request->IsA(UEnhancedOnlineRequest_CreateSession::StaticClass()))
		{
			HostOnlineSessionInternal(LocalPlayer, Cast<UEnhancedOnlineRequest_CreateSession>(Request));
		}
		else if (Request->IsA(UEnhancedOnlineRequest_CreateLobby::StaticClass()))
		{
			HostOnlineLobbyInternal(LocalPlayer, Cast<UEnhancedOnlineRequest_CreateLobby>(Request));
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("Host Online Session was called with a bad request."));
			Request->OnRequestFailedDelegate.Broadcast(TEXT("Host Online Session was called with a bad request."));
		}
	}
}


void UEnhancedOnlineSessionsSubsystem::HostOnlineLobbyInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateLobby* Request)
{
	PendingTravelURL = Request->GetTravelURL();

	check(Request->OnlineSub);
	check(Request->Sessions);

	FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();

	if (ensure(UserId.IsValid()))
	{
		HostLobbyDelegateHandle = Request->Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleHostOnlineLobbyComplete));
		
		SessionSettings = MakeShared<FEnhancedOnlineSessionSettings>(Request->OnlineMode == EEnhancedSessionOnlineMode::LAN, Request->bUsesPresence, Request->GetMaxPlayers(), Request->bAllowJoinInProgress);
		SessionSettings->bUseLobbiesIfAvailable = true;
		SessionSettings->bUseLobbiesVoiceChatIfAvailable = Request->bUseVoiceChatIfAvailable;
		SessionSettings->Set(SETTING_GAMEMODE, Request->GameModeAdvertisementName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MAPNAME, Request->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SEARCH_KEYWORDS, Request->SearchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
		SessionSettings->Set(SETTING_FRIENDLYNAME, Request->FriendlyName, EOnlineDataAdvertisementType::ViaOnlineService);

		FSessionSettings& UserSettings = SessionSettings->MemberSettings.Add(UserId.ToSharedRef(), FSessionSettings());
		UserSettings.Add(SETTING_GAMEMODE, FOnlineSessionSetting(FString("GameSession"), EOnlineDataAdvertisementType::ViaOnlineService));
		
		PendingSessionRequest = Request;

		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Hosting lobby with %d players..."), Request->GetMaxPlayers());

		if (!Request->Sessions->CreateSession(0, NAME_GameSession, *SessionSettings))
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to create lobby."));
			Request->OnRequestFailedDelegate.Broadcast(TEXT("Failed to create lobby."));

			/* Clear the delegate handle */
			Request->Sessions->ClearOnCreateSessionCompleteDelegate_Handle(HostLobbyDelegateHandle);
			HostLobbyDelegateHandle.Reset();
			PendingSessionRequest = nullptr;
		}
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleHostOnlineLobbyComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (bWasSuccessful)
	{
		if (PendingSessionRequest)
		{
			PendingSessionRequest->OnCreateSessionCompleted.Broadcast(0, SessionName);
		}
		
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Lobby created successfully."));
		
		if (!PendingTravelURL.ToString().IsEmpty())
		{
			GetWorld()->Listen(PendingTravelURL);
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("No travel URL was set."));
		}
	}
	else
	{
		if (PendingSessionRequest)
		{
			PendingSessionRequest->OnRequestFailedDelegate.Broadcast(TEXT("Failed to create lobby."));
		}
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to create lobby."));
	}

	/* Clear the delegate handle */
	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(HostLobbyDelegateHandle);
	HostLobbyDelegateHandle.Reset();

	if (PendingSessionRequest)
	{
		PendingSessionRequest->CompleteRequest();	
	}
	PendingSessionRequest = nullptr;
}

void UEnhancedOnlineSessionsSubsystem::HostOnlineSessionInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateSession* Request)
{
	PendingTravelURL = Request->GetTravelURL();

	check(Request->OnlineSub);
	check(Request->Sessions);

	FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();

	if (ensure(UserId.IsValid()))
	{
		HostSessionDelegateHandle = Request->Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleHostOnlineSessionComplete));

		SessionSettings = MakeShared<FEnhancedOnlineSessionSettings>(Request->OnlineMode == EEnhancedSessionOnlineMode::LAN, Request->bUsesPresence, Request->GetMaxPlayers(), Request->bAllowJoinInProgress);
		SessionSettings->bUseLobbiesIfAvailable = Request->bUseLobbiesIfAvailable;
		SessionSettings->bUseLobbiesVoiceChatIfAvailable = Request->bUseVoiceChatIfAvailable;
		SessionSettings->Set(SETTING_GAMEMODE, Request->GameModeAdvertisementName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MAPNAME, Request->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SEARCH_KEYWORDS, Request->SearchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
		SessionSettings->Set(SETTING_FRIENDLYNAME, Request->FriendlyName, EOnlineDataAdvertisementType::ViaOnlineService);

		FSessionSettings& UserSettings = SessionSettings->MemberSettings.Add(UserId.ToSharedRef(), FSessionSettings());
		UserSettings.Add(SETTING_GAMEMODE, FOnlineSessionSetting(Request->GameModeAdvertisementName, EOnlineDataAdvertisementType::ViaOnlineService));

		PendingSessionRequest = Request;

		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Hosting session with %d players..."), Request->GetMaxPlayers());

		if (!Request->Sessions->CreateSession(0, NAME_GameSession, *SessionSettings))
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to create session."));
			Request->OnRequestFailedDelegate.Broadcast(TEXT("Failed to create session."));

			/* Clear the delegate handle */
			Request->Sessions->ClearOnCreateSessionCompleteDelegate_Handle(HostSessionDelegateHandle);
			HostSessionDelegateHandle.Reset();
		}
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleHostOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (bWasSuccessful)
	{
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Session created successfully."));

		if (!PendingTravelURL.ToString().IsEmpty())
		{
			GetWorld()->ServerTravel(PendingTravelURL.ToString());	
		}
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to create session."));

		if (PendingSessionRequest)
		{
			PendingSessionRequest->OnRequestFailedDelegate.Broadcast(TEXT("Failed to create session."));
		}
	}

	/* Clear the delegate handle */
	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(HostSessionDelegateHandle);
	HostSessionDelegateHandle.Reset();

	if (PendingSessionRequest)
	{
		PendingSessionRequest->CompleteRequest();
		PendingSessionRequest = nullptr;
	}
}

void UEnhancedOnlineSessionsSubsystem::FindOnlineSessions(UEnhancedOnlineRequest_FindSessions* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Online Sessions was called with a bad request."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Online Sessions was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Find Online Sessions was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Online Sessions was called with a bad local user index: %d."), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Find Online Sessions was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	FindOnlineSessionsInternal(LocalPlayer, MakeShared<FEnhancedOnlineSearchSettings>(Request));
}

void UEnhancedOnlineSessionsSubsystem::FindOnlineSessionsInternal(ULocalPlayer* LocalPlayer, const TSharedRef<FEnhancedOnlineSearchSettings>& InSearchSettings)
{
	if (SearchSettings.IsValid())
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("A search is already in progress."));
		InSearchSettings->Request->OnRequestFailedDelegate.Broadcast(TEXT("A search is already in progress."));
		return;
	}

	SearchSettings = InSearchSettings;

	FindSessionsDelegateHandle = SearchSettings->Request->Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindOnlineSessionsComplete));

	if (!InSearchSettings->Request->Sessions->FindSessions(0, InSearchSettings))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to find sessions. :("));
		InSearchSettings->Request->OnRequestFailedDelegate.Broadcast(TEXT("Failed to find sessions. :("));

		InSearchSettings->Request->Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		FindSessionsDelegateHandle.Reset();
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleFindOnlineSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Found sessions successfully."));

		if (SearchSettings.IsValid())
		{
			TArray<UEnhancedSessionSearchResult*> Results;
			for (auto& SearchResult : SearchSettings->SearchResults)
			{
				UEnhancedSessionSearchResult* NewResult = NewObject<UEnhancedSessionSearchResult>(SearchSettings->Request);
				NewResult->StoredSearchResult = SearchResult;
				Results.Add(NewResult);

				FString OwningUserId = TEXT("Uknown");
				if (SearchResult.Session.OwningUserId.IsValid())
				{
					OwningUserId = SearchResult.Session.OwningUserId->ToString();
				}
				
				UE_LOG(LogEnhancedSubsystem, Log, TEXT("\tFound session (UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
				*OwningUserId,
				*SearchResult.Session.OwningUserName,
				SearchResult.Session.NumOpenPrivateConnections,
				SearchResult.Session.NumOpenPublicConnections,
				SearchResult.PingInMs);
			}

			SearchSettings->Request->OnFindOnlineSessionsCompleted.Broadcast(Results);
		}
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to find sessions. :("));
	}

	if (SearchSettings.IsValid())
	{
		SearchSettings->Request->Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		FindSessionsDelegateHandle.Reset();
		
		SearchSettings->Request->CompleteRequest();
		SearchSettings = nullptr;
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Invalid search settings. Did we lose a reference? :("));
	}
}

void UEnhancedOnlineSessionsSubsystem::JoinOnlineSession(UEnhancedOnlineRequest_JoinSession* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Join Online Session was called with a bad search result."));
		return;
	}

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	JoinSessionDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoinSessionCompleted));

	Sessions->GetResolvedConnectString(Request->SessionToJoin->StoredSearchResult, NAME_GamePort, PendingClientTravelURL);

	if (!Sessions->JoinSession(0, NAME_GameSession, Request->SessionToJoin->StoredSearchResult))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to join session."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Failed to join session."));

		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
		JoinSessionDelegateHandle.Reset();
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Joined session successfully."));

		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController == nullptr)
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to get player controller."));
			return;
		}

		PlayerController->ClientTravel(PendingClientTravelURL, TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to join session."));
	}

	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
	JoinSessionDelegateHandle.Reset();
}

void UEnhancedOnlineSessionsSubsystem::StartOnlineSession(UEnhancedOnlineRequest_StartSession* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Start Online Session was called with a bad request."));
		return;
	}

	IOnlineSessionPtr Sessions = Request->Sessions;

	if (Sessions == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Start Online Session was called with a bad session interface."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Start Online Session was called with a bad session interface."));
		return;
	}

	StartSessionDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleStartOnlineSessionComplete));
	PendingStartSessionRequest = Request;

	if (!Sessions->StartSession(NAME_GameSession))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to start session."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Failed to start session."));

		Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartSessionDelegateHandle);
		StartSessionDelegateHandle.Reset();
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleStartOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (PendingStartSessionRequest == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Start Online Session was called with a bad request."));
		return;
	}

	if (bWasSuccessful)
	{
		PendingStartSessionRequest->OnStartSessionCompleted.Broadcast(SessionName, true);
	}
	else
	{
		PendingStartSessionRequest->OnRequestFailedDelegate.Broadcast(TEXT("Failed to start session."));
	}

	PendingStartSessionRequest->Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartSessionDelegateHandle);
	StartSessionDelegateHandle.Reset();
}