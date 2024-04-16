// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineSessions.h"
#include "EnhancedOnlineSubsystem.h"
#include "EnhancedOnlineTypes.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"

void UEnhancedOnlineSubsystem::BindSessionDelegates(IOnlineSessionPtr Sessions)
{
	check(Sessions.IsValid());

	Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	Sessions->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));
}


void UEnhancedOnlineSubsystem::CreateOnlineSession(UEnhancedOnlineRequest_CreateSession* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Create Online Session was called with a bad request."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Create Online Session was called with a bad local player index."));
		Request->OnRequestFailed.Broadcast(TEXT("Create Online Session was called with a bad local player index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Create Online Session is unable to get the local player of the local user index: %d"), Request->LocalUserIndex);
		Request->OnRequestFailed.Broadcast(TEXT("Create Online Session is unable to get the local player of the local user index."));
		return;
	}

	if (Request->OnlineMode == EEnhancedSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			Request->OnRequestFailed.Broadcast(TEXT("Cannot create a session in offline mode on a client."));
			return;
		}

		GetWorld()->ServerTravel(Request->ConstructTravelURL(Request->bUseServerTravelOnSuccess));
	}
	else
	{
		CreateOnlineSessionInternal(LocalPlayer, Request);
	}
}

void UEnhancedOnlineSubsystem::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_CreateSession* Request)
{
	PendingTravelURL = Request->ConstructTravelURL();

	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();
	const bool bUsesPresence = Request->bUseLobbiesIfAvailable;

	check(Request->OnlineSub);
	check(Request->Sessions);

	FUniqueNetIdPtr UserId;
	if (LocalPlayer)
	{
		UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	}

	if (ensure(UserId.IsValid()))
	{
		SessionSettings = MakeShareable(new FEnhancedOnlineSessionSettings(Request->OnlineMode == EEnhancedSessionOnlineMode::LAN, bUsesPresence, MaxPlayers));
		SessionSettings->bUseLobbiesIfAvailable = Request->bUseLobbiesIfAvailable;
		SessionSettings->bUseLobbiesVoiceChatIfAvailable = Request->bUseVoiceChatIfAvailable;
		SessionSettings->Set(SETTING_GAMEMODE, Request->AdvertisementGameModeName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MAPNAME, Request->GetFullMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")), EOnlineDataAdvertisementType::DontAdvertise);
		SessionSettings->Set(SEARCH_KEYWORDS, Request->SearchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

		FSessionSettings& UserSettings = SessionSettings->MemberSettings.Add(UserId.ToSharedRef(), FSessionSettings());
		UserSettings.Add(SETTING_GAMEMODE, FOnlineSessionSetting(FString("GameSession"), EOnlineDataAdvertisementType::ViaOnlineService));

		Request->Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
	}
	else
	{
		Request->OnRequestFailed.Broadcast(TEXT("Failed to get a valid user id. Is the user logged in?"));
	}
}

void UEnhancedOnlineSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogEnhancedSubsystem, Log, TEXT("OnCreateSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

	if (bWasSuccessful)
	{
		IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
		check(OnlineSub);

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		check(Sessions.IsValid());

		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Starting session: %s"), *SessionName.ToString());
		Sessions->StartSession(SessionName);
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());
	}
}

void UEnhancedOnlineSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(PendingTravelURL);
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Failed to start session: %s"), *SessionName.ToString());
	}
}
