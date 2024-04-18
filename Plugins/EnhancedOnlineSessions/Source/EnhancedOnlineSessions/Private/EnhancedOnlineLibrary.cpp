// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineLibrary.h"

#include "EnhancedOnlineRequests.h"
#include "EnhancedOnlineSessions.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UEnhancedOnlineLibrary::SetupFailureDelegate(
	UEnhancedOnlineRequest* Request,
	FBlueprintOnRequestFailedWithLogin OnFailed)
{
	Request->OnRequestFailed.AddLambda(
		[OnFailed, Request](const FString& ErrorMessage)
		{
			if (OnFailed.IsBound())
			{
				OnFailed.Execute(ErrorMessage);
			}
			Request->InvalidateRequest();
		});
}

void UEnhancedOnlineLibrary::SwitchOnLoginStatus(EEnhancedLoginStatus& LoginStatus, UObject* WorldContextObject,int32 LocalUserIndex)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(WorldContextObject->GetWorld());
	check(OnlineSub);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("SwitchOnLoginStatus was called with a bad local player index."));
		LoginStatus = EEnhancedLoginStatus::NotLoggedIn;
		return;
	}

	ELoginStatus::Type PlayerLoginStatus = Identity->GetLoginStatus(0);
	LoginStatus = static_cast<EEnhancedLoginStatus>(static_cast<int32>(PlayerLoginStatus));
}

bool UEnhancedOnlineLibrary::IsPlayerLoggedIn(UObject* WorldContextObject, int32 LocalUserIndex)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(WorldContextObject->GetWorld());
	check(OnlineSub);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	return Identity->GetLoginStatus(LocalUserIndex) == ELoginStatus::LoggedIn;
}

UEnhancedOnlineRequest_LoginUser* UEnhancedOnlineLibrary::ConstructOnlineLoginUserRequest(
	UObject* WorldContextObject, const FString UserID, const FString UserToken, const EEnhancedAuthType AuthType, int32 LocalUserIndex, bool bInvalidateAfterComplete,
	FBlueprintOnLoginUserSuccess OnSuccess, FBlueprintOnRequestFailedWithLogin OnFailed)
{
	UEnhancedOnlineRequest_LoginUser* Request = NewObject<UEnhancedOnlineRequest_LoginUser>(WorldContextObject);
	Request->bInvalidateAfterComplete = bInvalidateAfterComplete;
	Request->LocalUserIndex = LocalUserIndex;
	Request->UserId = UserID;
	Request->UserToken = UserToken;
	Request->AuthType = AuthType;

	Request->ConstructRequest();
	SetupFailureDelegate(Request, OnFailed);

	Request->OnLoginSuccess.AddLambda(
		[OnSuccess](int32 LocalPlayerIndex)
		{
			OnSuccess.Execute(LocalPlayerIndex);
		});

	return Request;
}

UEnhancedOnlineRequest_CreateSession* UEnhancedOnlineLibrary::ConstructOnlineCreateSessionRequest(
	UObject* WorldContextObject, const EEnhancedSessionOnlineMode OnlineMode, int32 MaxPlayerCount,
	FPrimaryAssetId MapId, FString SessionFriendlyName, FString SearchKeyword, bool bUseLobbiesIfAvailable,
	bool bUseVoiceChatIfAvailable, bool bUseServerTravelOnSuccess, FString AdvertisementGameModeName, const TArray<FEnhancedStoredExtraSessionSettings>& StoredSettings,
	int32 LocalUserIndex, bool bInvalidateAfterComplete, FBlueprintOnRequestFailedWithLogin OnFailed)
{
	UEnhancedOnlineRequest_CreateSession* Request = NewObject<UEnhancedOnlineRequest_CreateSession>(WorldContextObject);
	Request->bInvalidateAfterComplete = bInvalidateAfterComplete;
	Request->LocalUserIndex = LocalUserIndex;
	Request->OnlineMode = OnlineMode;
	Request->MaxPlayerCount = MaxPlayerCount;
	Request->MapId = MapId;
	Request->SessionFriendlyName = SessionFriendlyName;
	Request->SearchKeyword = SearchKeyword;
	Request->StoredSettings = StoredSettings;
	Request->bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
	Request->bUseVoiceChatIfAvailable = bUseVoiceChatIfAvailable;
	Request->bUseServerTravelOnSuccess = bUseServerTravelOnSuccess;
	Request->AdvertisementGameModeName = AdvertisementGameModeName;

	Request->ConstructRequest();
	SetupFailureDelegate(Request, OnFailed);

	return Request;
}

UEnhancedOnlineRequest_SearchSessions* UEnhancedOnlineLibrary::ConstructOnlineSearchSessionsRequest(
	UObject* WorldContextObject, const EEnhancedSessionOnlineMode OnlineMode, int32 MaxSearchResults,
	bool bSearchLobbiesIfAvailable, FString SearchKeyword, int32 LocalUserIndex, bool bInvalidateAfterComplete,
	FBlueprintOnFindSessionsSuccess OnSuccess, FBlueprintOnRequestFailedWithLogin OnFailed)
{
	UEnhancedOnlineRequest_SearchSessions* Request = NewObject<UEnhancedOnlineRequest_SearchSessions>(WorldContextObject);
	Request->bInvalidateAfterComplete = bInvalidateAfterComplete;
	Request->LocalUserIndex = LocalUserIndex;
	Request->OnlineMode = OnlineMode;
	Request->MaxSearchResults = MaxSearchResults;
	Request->SearchKeyword = SearchKeyword;
	Request->bUseLobbiesIfAvailable = bSearchLobbiesIfAvailable;

	Request->ConstructRequest();
	SetupFailureDelegate(Request, OnFailed);

	Request->OnFindSessionsSuccess.AddLambda(
		[OnSuccess](const TArray<UEnhancedSessionSearchResult*>& SearchResults)
		{
			OnSuccess.Execute(SearchResults);
		});

	return Request;
}

UEnhancedOnlineRequest_JoinSession* UEnhancedOnlineLibrary::ConstructOnlineJoinSessionRequest(
	UObject* WorldContextObject, UEnhancedSessionSearchResult* SessionToJoin, int32 LocalUserIndex,
	bool bInvalidateAfterComplete, FBlueprintOnRequestFailedWithLogin OnFailed)
{
	UEnhancedOnlineRequest_JoinSession* Request = NewObject<UEnhancedOnlineRequest_JoinSession>(WorldContextObject);
	Request->bInvalidateAfterComplete = bInvalidateAfterComplete;
	Request->LocalUserIndex = LocalUserIndex;
	Request->SessionToJoin = SessionToJoin;

	Request->ConstructRequest();
	SetupFailureDelegate(Request, OnFailed);

	return Request;
}

FEnhancedStoredExtraSessionSettings UEnhancedOnlineLibrary::MakeSettingByInt(const FName Key, const int32 Value)
{
	return MakeSetting<int32>(Key, Value);
}

FEnhancedStoredExtraSessionSettings UEnhancedOnlineLibrary::MakeSettingByString(const FName Key, const FString Value)
{
	return MakeSetting<FString>(Key, Value);
}

FEnhancedStoredExtraSessionSettings UEnhancedOnlineLibrary::MakeSettingByFloat(const FName Key, const float Value)
{
	return MakeSetting<float>(Key, Value);
}

FEnhancedStoredExtraSessionSettings UEnhancedOnlineLibrary::MakeSettingByBool(const FName Key, const bool Value)
{
	return MakeSetting<bool>(Key, Value);
}

FUniqueNetIdRepl UEnhancedOnlineLibrary::GetUniqueNetIdFromPlayerController(APlayerController* PlayerController)
{
	APlayerState* PlayerState = PlayerController->PlayerState;
	return GetUniqueNetIdFromPlayerState(PlayerState);
}

FUniqueNetIdRepl UEnhancedOnlineLibrary::GetUniqueNetIdFromPlayerState(APlayerState* PlayerState)
{
	if (PlayerState == nullptr)
	{
		return FUniqueNetIdRepl();
	}

	return PlayerState->GetUniqueId().GetUniqueNetId();
}

FUniqueNetIdRepl UEnhancedOnlineLibrary::GetUniqueNetIdEOS(UObject* WorldContextObject, const int32 LocalPlayerIndex)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GetWorldFromContextObjectChecked(WorldContextObject), LocalPlayerIndex);
	
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(PlayerController->GetWorld());
	check(OnlineSub);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	return FUniqueNetIdRepl(Identity->GetUniquePlayerId(LocalPlayerIndex));
}

FString UEnhancedOnlineLibrary::GetPlayerNicknameEOS(UObject* WorldContextObject, const int32 LocalPlayerIndex)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GetWorldFromContextObjectChecked(WorldContextObject), LocalPlayerIndex);
	
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(PlayerController->GetWorld());
	check(OnlineSub);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	return Identity->GetPlayerNickname(0);
}

FString UEnhancedOnlineLibrary::UniqueNetIdToString(const FUniqueNetIdRepl& UniqueNetId)
{
	return UniqueNetId.GetUniqueNetId()->ToString();
}

