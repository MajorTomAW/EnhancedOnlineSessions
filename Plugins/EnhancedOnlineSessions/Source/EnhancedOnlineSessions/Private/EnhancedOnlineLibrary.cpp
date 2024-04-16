// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineLibrary.h"

#include "EnhancedOnlineRequests.h"

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
	FPrimaryAssetId MapId, bool bUseLobbiesIfAvailable, FString AdvertisementGameModeName, int32 LocalUserIndex,
	bool bInvalidateAfterComplete, FBlueprintOnRequestFailedWithLogin OnFailed)
{
	UEnhancedOnlineRequest_CreateSession* Request = NewObject<UEnhancedOnlineRequest_CreateSession>(WorldContextObject);
	Request->bInvalidateAfterComplete = bInvalidateAfterComplete;
	Request->LocalUserIndex = LocalUserIndex;
	Request->OnlineMode = OnlineMode;
	Request->MaxPlayerCount = MaxPlayerCount;
	Request->MapId = MapId;
	Request->bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
	Request->AdvertisementGameModeName = AdvertisementGameModeName;

	Request->ConstructRequest();
	SetupFailureDelegate(Request, OnFailed);

	return Request;
}
