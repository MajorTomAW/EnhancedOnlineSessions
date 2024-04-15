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

/************************************************************************************
 * Common Online Session Requests													*
 ************************************************************************************/
#pragma region create_session_request
int32 UCommonOnline_CreateSessionRequest::GetMaxPlayers() const
{
	return MaxPlayerCount;
}

FString UCommonOnline_CreateSessionRequest::GetMapName() const
{
	FAssetData MapAssetData;
	if (UAssetManager::Get().GetPrimaryAssetData(MapID, MapAssetData))
	{
		return MapAssetData.PackageName.ToString();
	}
	else
	{
		return FString();
	}
}

FString UCommonOnline_CreateSessionRequest::ConstructTravelURL() const
{
	FString CombinedExtraArgs;

	if (OnlineMode == ECommonSessionOnlineMode::LAN)
	{
		CombinedExtraArgs += TEXT("?bIsLanMatch");
	}

	if (OnlineMode != ECommonSessionOnlineMode::Offline)
	{
		CombinedExtraArgs += TEXT("?listen");
	}

	return FString::Printf(TEXT("%s%s"),
		*GetMapName(),
		*CombinedExtraArgs);
}

bool UCommonOnline_CreateSessionRequest::ValidateAndLogErrors(FText& OutError) const
{
#if WITH_SERVER_CODE
	if (GetMapName().IsEmpty())
	{
		OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed"), FText::FromString(MapID.ToString()));
		return false;
	}
	return true;
#else
	OutError = NSLOCTEXT("NetworkErrors", "ClientBuildCannotHost", "Client builds cannot host game sessions.");
	return false;
#endif
}
#pragma endregion

#pragma region find_session_request
int32 UCommonOnlineSearchResult::GetPingInMs() const
{
	return Result.PingInMs;
}

int32 UCommonOnlineSearchResult::GetNumCurrentPlayers() const
{
	return Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
}

int32 UCommonOnlineSearchResult::GetNumMaxPlayers() const
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

FString UCommonOnlineSearchResult::GetSessionNameSafe() const
{
	if (Result.Session.OwningUserId.IsValid())
	{
		if (IOnlineIdentityPtr IdentityPtr = Online::GetIdentityInterface(GetWorld()))
		{
			return IdentityPtr->GetPlayerNickname(*Result.Session.OwningUserId.Get());
		}
	}

	if (!Result.Session.OwningUserName.IsEmpty())
	{
		return Result.Session.OwningUserName;
	}

	if (Result.Session.SessionSettings.Settings.Array().Num() > 0)
	{
		for (auto& Setting : Result.Session.SessionSettings.Settings)
		{
			if (Setting.Key == "FRIENDLYNAME")
			{
				FString OwnerName;
				Setting.Value.Data.GetValue(OwnerName);
				return OwnerName;
			}
		}
	}

	return TEXT("Unknown");
}
#pragma endregion

/************************************************************************************
 * UCommonOnlineSubsystem															*
 ************************************************************************************/

UCommonOnlineSubsystem::UCommonOnlineSubsystem()
{
}

void UCommonOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
}

void UCommonOnlineSubsystem::BindOnlineDelegates()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	const IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Sessions.IsValid() && Identity.IsValid());

	Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));

	// Bind to the login complete delegate for all local players
	for (int32 PlayerIndex = 0; PlayerIndex < MAX_LOCAL_PLAYERS; PlayerIndex++)
	{
		Identity->AddOnLoginCompleteDelegate_Handle(PlayerIndex, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));	
	}
}

void UCommonOnlineSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UCommonOnlineSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> SubClasses;
	GetDerivedClasses(GetClass(), SubClasses, false);

	// Only create an instance if this is the only derived class
	return SubClasses.Num() == 0;
}

#pragma region login_online_user
void UCommonOnlineSubsystem::LoginOnlineUser(APlayerController* PlayerToLogin, UCommonOnline_LoginUserRequest* LoginRequest)
{
	if (LoginRequest == nullptr)
	{
		LoginRequest->OnFailed.Broadcast(TEXT("Login Online User was called with a bad request"));
		return;
	}

	ULocalPlayer* LocalPlayer = (PlayerToLogin != nullptr) ? PlayerToLogin->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		LoginRequest->OnFailed.Broadcast(TEXT("Unable to find local player"));
	}

	LoginOnlineUserInternal(LocalPlayer, LoginRequest);
}

void UCommonOnlineSubsystem::LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LoginUserRequest* LoginRequest)
{
	const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub)

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	EAuthType AuthType = LoginRequest->AuthType;
	FString AuthTypeString;
	StaticEnum<EAuthType>()->FindNameStringByValue(AuthTypeString, static_cast<int64>(AuthType));

	FOnlineAccountCredentials Credentials;
	Credentials.Type = AuthTypeString;
	Credentials.Id = LoginRequest->UserId;
	Credentials.Token = LoginRequest->UserToken;

	Identity->OnLoginCompleteDelegates->AddLambda(
		[this, LoginRequest] (int32 LocalUserIndex, bool bWasSuccessful, const FUniqueMessageId& UserId, const FString& Error)
		{
			if (bWasSuccessful)
			{
				LoginRequest->OnLoginUserSuccess.Broadcast(LocalUserIndex, UserId);
			}
			else
			{
				LoginRequest->OnFailed.Broadcast(Error);
			}
		});

	Identity->Login(LocalPlayer->GetControllerId(), Credentials);
}

void UCommonOnlineSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
}
#pragma endregion

#pragma region create_online_session
void UCommonOnlineSubsystem::CreateOnlineSession(APlayerController* HostingPlayer, UCommonOnline_CreateSessionRequest* CreateSessionRequest)
{
	if (CreateSessionRequest == nullptr)
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Create Online Session was called with a bad request"));
		return;
	}

	ULocalPlayer* LocalPlayer = (HostingPlayer != nullptr) ? HostingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		CreateSessionRequest->OnFailed.Broadcast(TEXT("Unable to find local player"));
		return;
	}

	FText OutError;
	if (!CreateSessionRequest->ValidateAndLogErrors(OutError))
	{
		CreateSessionRequest->OnFailed.Broadcast(OutError.ToString());
		return;
	}

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
	PendingTravelURL = CreateSessionRequest->ConstructTravelURL();

	const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

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
	if (FindSessionsRequest == nullptr)
	{
		FindSessionsRequest->OnFailed.Broadcast(TEXT("Find Online Sessions was called with a bad request"));
		return;
	}

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