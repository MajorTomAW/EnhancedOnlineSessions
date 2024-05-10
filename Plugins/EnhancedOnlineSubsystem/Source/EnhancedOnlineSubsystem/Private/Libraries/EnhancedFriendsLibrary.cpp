// Copyright © 2024 MajorT. All rights reserved.


#include "Libraries/EnhancedFriendsLibrary.h"

UEnhancedOnlineRequest_GetFriendsList* UEnhancedFriendsLibrary::ConstructOnlineGetFriendsListRequest(
	UObject* WorldContextObject, const bool bFilterOnlyOnlineFriends, const bool bFilterOnlyInGameFriends,
	const int32 LocalUserIndex, const bool bInvalidateOnCompletion,
	FBPOnGetFriendsListCompleted OnCompletedDelegate, FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_GetFriendsList* Request = NewObject<UEnhancedOnlineRequest_GetFriendsList>(WorldContextObject);
	Request->ConstructRequest();
	
	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->bFilterOnlyOnlineFriends = bFilterOnlyOnlineFriends;
	Request->bFilterOnlyInGameFriends = bFilterOnlyInGameFriends;
	
	SetupFailureDelegate(Request, OnFailedDelegate);
	Request->OnGetFriendsListCompleted.AddLambda(
		[OnCompletedDelegate] (const TArray<FEnhancedBlueprintFriendInfo>& FriendsList)
		{
			if (OnCompletedDelegate.IsBound())
			{
				OnCompletedDelegate.Execute(FriendsList);
			}
		});

	return Request;
}


UEnhancedOnlineRequest_FindFriendSession* UEnhancedFriendsLibrary::ConstructOnlineFindFriendSessionRequest(
	UObject* WorldContextObject, const FUniqueNetIdRepl& FriendId, const int32 LocalUserIndex,
	const bool bInvalidateOnCompletion, FBPOnFindSessionsSuceeeded OnCompletedDelegate,
	FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_FindFriendSession* Request = NewObject<UEnhancedOnlineRequest_FindFriendSession>(WorldContextObject);
	Request->ConstructRequest();

	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->FriendId = FriendId;

	SetupFailureDelegate(Request, OnFailedDelegate);
	Request->OnFindFriendSessionCompleted.AddLambda(
		[OnCompletedDelegate] (const TArray<UEnhancedSessionSearchResult*>& SearchResults)
		{
			if (OnCompletedDelegate.IsBound())
			{
				OnCompletedDelegate.Execute(SearchResults);
			}
		});

	return Request;
}
