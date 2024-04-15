// Copyright © 2024 MajorT. All rights reserved.


#include "BlueprintLibrary/CommonOnlineSessionsLibrary.h"

#include "CommonOnlineSubsystem.h"

UCommonOnline_LoginUserRequest* UCommonOnlineSessionsLibrary::ConstructOnlineLoginUserRequest(
	UObject* WorldContextObject, const FString UserID, const FString UserToken,
	const EAuthType AuthType, FBlueprintOnLoginSuccess OnSuccess, FBlueprintOnRequestFailedWithLog OnFailed)
{
	UCommonOnline_LoginUserRequest* Request = NewObject<UCommonOnline_LoginUserRequest>(WorldContextObject);
	Request->UserId = UserID;
	Request->UserToken = UserToken;
	Request->AuthType = AuthType;

	Request->OnFailed.AddLambda(
		[OnFailed, Request] (const FString& ErrorLog)
		{
			if (OnFailed.IsBound())
			{
				OnFailed.Execute(ErrorLog);
			}
			Request->InvalidateRequest();
		});

	Request->OnLoginUserSuccess.AddLambda(
		[OnSuccess] (int32 LocalUserIndex, const FUniqueMessageId& MessageId)
		{
			OnSuccess.Execute(LocalUserIndex);
		});
	
	return Request;
}

UCommonOnline_CreateSessionRequest* UCommonOnlineSessionsLibrary::ConstructOnlineCreateSessionRequest(
	UObject* WorldContextObject, int32 MaxPlayerCount, FPrimaryAssetId MapID, ECommonSessionOnlineMode OnlineMode,
	FString SessionFriendlyName, FString GameModeFriendlyName, FString SearchKeyword, bool bUseLobbiesIfAvailable, bool bUseVoiceChatIfAvailable,
	FStoredSessionSettings StoredSettings, FBlueprintOnRequestFailedWithLog OnFailed)
{
	UCommonOnline_CreateSessionRequest* Request = NewObject<UCommonOnline_CreateSessionRequest>(WorldContextObject);
	Request->MaxPlayerCount = MaxPlayerCount;
	Request->MapID = MapID;
	Request->OnlineMode = OnlineMode;
	Request->SessionFriendlyName = SessionFriendlyName;
	Request->SearchKeyword = SearchKeyword;
	Request->GameModeFriendlyName = GameModeFriendlyName;
	Request->bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
	Request->bUseVoiceChatIfAvailable = bUseVoiceChatIfAvailable;
	Request->StoredSettings = StoredSettings;

	Request->OnFailed.AddLambda(
		[OnFailed, Request] (const FString& ErrorLog)
		{
			if (OnFailed.IsBound())
			{
				OnFailed.Execute(ErrorLog);
			}
			Request->InvalidateRequest();
		});

	return Request;
}

bool UCommonOnlineSessionsLibrary::InvalidateRequest(UCommonOnlineRequest* Request)
{
	if (Request == nullptr)
	{
		return false;
	}

	Request->InvalidateRequest();
	return true;
}

UCommonOnline_FindSessionsRequest* UCommonOnlineSessionsLibrary::ConstructOnlineFindSessionsRequest(
	UObject* WorldContextObject, int32 MaxNumResults, ECommonSessionOnlineMode OnlineMode, bool bSearchLobbies,
	FBlueprintOnOnFindSessionsSuccess OnSuccess, FBlueprintOnRequestFailedWithLog OnFailed)
{
	UCommonOnline_FindSessionsRequest* Request = NewObject<UCommonOnline_FindSessionsRequest>(WorldContextObject);
	Request->MaxSearchResults = MaxNumResults > 0 ? MaxNumResults : INT_MAX;
	Request->OnlineMode = OnlineMode;
	Request->bSearchLobbies = bSearchLobbies;

	Request->OnFailed.AddLambda(
		[OnFailed, Request] (const FString& ErrorLog)
		{
			if (OnFailed.IsBound())
			{
				OnFailed.Execute(ErrorLog);
			}
			Request->InvalidateRequest();
		});

	Request->OnFindSessionsSuccess.AddLambda(
		[OnSuccess] (const TArray<UCommonOnlineSearchResult*>& SearchResults)
		{
			if (OnSuccess.IsBound())
			{
				OnSuccess.Execute(SearchResults);
			}
		});
	
	return Request;
}
