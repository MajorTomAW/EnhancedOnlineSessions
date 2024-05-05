// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedSessionsLibrary.h"

#include "EnhancedOnlineRequests.h"


void UEnhancedSessionsLibrary::SetupFailureDelegate(UEnhancedOnlineRequestBase* Request, FBPOnRequestFailedWithLog OnFailedDelegate)
{
	Request->OnRequestFailedDelegate.AddLambda(
		[OnFailedDelegate, Request] (const FString& Reason)
		{
			if (OnFailedDelegate.IsBound())
			{
				OnFailedDelegate.Execute(Reason);
			}

			Request->InvalidateRequest();
		});
}

int32 UEnhancedSessionsLibrary::GetPingInMs(UEnhancedSessionSearchResult* SearchResult)
{
	return SearchResult->GetPingInMs();
}

int32 UEnhancedSessionsLibrary::GetMaxPlayers(UEnhancedSessionSearchResult* SearchResult)
{
	return SearchResult->GetMaxPlayers();
}

int32 UEnhancedSessionsLibrary::GetCurrentPlayers(UEnhancedSessionSearchResult* SearchResult)
{
	return SearchResult->GetCurrentPlayers();
}

FString UEnhancedSessionsLibrary::GetSessionFriendlyName(UEnhancedSessionSearchResult* SearchResult)
{
	return SearchResult->GetSessionFriendlyName();
}

UEnhancedOnlineRequest_CreateSession* UEnhancedSessionsLibrary::ConstructOnlineHostSessionRequest(
	UObject* WorldContextObject, const EEnhancedSessionOnlineMode OnlineMode, const int32 MaxPlayerCount,
	FPrimaryAssetId MapId, const FString FriendlyName, const FString SearchKeyword, const bool bUseLobbiesIfAvailable,
	const bool bUseVoiceChatIfAvailable, const FString GameModeAdvertisementName, const bool bIsPresence, const bool bAllowJoinInProgress,
	const int32 LocalUserIndex, const bool bInvalidateOnCompletion, FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_CreateSession* Request = NewObject<UEnhancedOnlineRequest_CreateSession>(WorldContextObject);
	Request->ConstructRequest();

	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->OnlineMode = OnlineMode;
	Request->MaxPlayerCount = MaxPlayerCount;
	Request->MapId = MapId;
	Request->FriendlyName = FriendlyName;
	Request->SearchKeyword = SearchKeyword;
	Request->bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
	Request->bUseVoiceChatIfAvailable = bUseVoiceChatIfAvailable;
	Request->GameModeAdvertisementName = GameModeAdvertisementName;
	Request->bUsesPresence = bIsPresence;
	Request->bAllowJoinInProgress = bAllowJoinInProgress;

	SetupFailureDelegate(Request, OnFailedDelegate);
	
	return Request;
}

UEnhancedOnlineRequest_CreateLobby* UEnhancedSessionsLibrary::ConstructOnlineHostLobbyRequest(
	UObject* WorldContextObject, const EEnhancedSessionOnlineMode OnlineMode, const int32 MaxPlayerCount,
	const FString FriendlyName, const FString SearchKeyword, const bool bUseVoiceChatIfAvailable,
	const FString GameModeAdvertisementName, const bool bIsPresence, const bool bAllowJoinInProgress, const int32 LocalUserIndex,
	bool bInvalidateOnCompletion, FBPOnHostLobbyRequestSucceeded OnSucceededDelegate,
	FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_CreateLobby* Request = NewObject<UEnhancedOnlineRequest_CreateLobby>(WorldContextObject);
	Request->ConstructRequest();

	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->OnlineMode = OnlineMode;
	Request->MaxPlayerCount = MaxPlayerCount;
	Request->FriendlyName = FriendlyName;
	Request->SearchKeyword = SearchKeyword;
	Request->bUseLobbiesIfAvailable = true;
	Request->bUseVoiceChatIfAvailable = bUseVoiceChatIfAvailable;
	Request->GameModeAdvertisementName = GameModeAdvertisementName;
	Request->bUsesPresence = bIsPresence;
	Request->bAllowJoinInProgress = bAllowJoinInProgress;

	SetupFailureDelegate(Request, OnFailedDelegate);

	Request->OnCreateSessionCompleted.AddLambda(
		[OnSucceededDelegate, Request] (int32 LocalUserIndex, const FName SessionName)
		{
			if (OnSucceededDelegate.IsBound())
			{
				OnSucceededDelegate.Execute(SessionName, LocalUserIndex);
			}

			Request->CompleteRequest();
		});

	return Request;
}

UEnhancedOnlineRequest_FindSessions* UEnhancedSessionsLibrary::ConstructOnlineFindSessionsRequest(
	UObject* WorldContextObject, const EEnhancedSessionOnlineMode OnlineMode, const int32 MaxSearchResults,
	const bool bFindLobbies, const FString SearchKeyword, const int32 LocalUserIndex,
	const bool bInvalidateOnCompletion, FBPOnFindSessionsSuceeeded OnSucceededDelegate,
	FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_FindSessions* Request = NewObject<UEnhancedOnlineRequest_FindSessions>(WorldContextObject);
	Request->ConstructRequest();

	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->OnlineMode = OnlineMode;
	Request->MaxSearchResults = MaxSearchResults;
	Request->bFindLobbies = bFindLobbies;
	Request->SearchKeyword = SearchKeyword;

	SetupFailureDelegate(Request, OnFailedDelegate);

	Request->OnFindOnlineSessionsCompleted.AddLambda(
		[OnSucceededDelegate, Request] ( const TArray<UEnhancedSessionSearchResult*>& SearchResults)
		{
			if (OnSucceededDelegate.IsBound())
			{
				OnSucceededDelegate.Execute(SearchResults);
			}

			Request->CompleteRequest();
		});

	return Request;
}

UEnhancedOnlineRequest_JoinSession* UEnhancedSessionsLibrary::ConstructOnlineJoinSessionRequest(
	UObject* WorldContextObject, UEnhancedSessionSearchResult* SessionToJoin, const int32 LocalUserIndex,
	const bool bInvalidateOnCompletion, FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_JoinSession* Request = NewObject<UEnhancedOnlineRequest_JoinSession>(WorldContextObject);
	Request->ConstructRequest();

	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;
	Request->SessionToJoin = SessionToJoin;

	SetupFailureDelegate(Request, OnFailedDelegate);
	return Request;
}

UEnhancedOnlineRequest_StartSession* UEnhancedSessionsLibrary::ContstructOnlineStartSessionRequest(
	UObject* WorldContextObject, const bool bInvalidateOnCompletion,
	FBPOnStartSessionRequestSucceeded OnSucceededDelegate, FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_StartSession* Request = NewObject<UEnhancedOnlineRequest_StartSession>(WorldContextObject);
	Request->ConstructRequest();

	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;

	SetupFailureDelegate(Request, OnFailedDelegate);

	Request->OnStartSessionCompleted.AddLambda(
		[OnSucceededDelegate, Request] (FName SessionName, bool bWasSuccessful)
		{
			if (OnSucceededDelegate.IsBound())
			{
				OnSucceededDelegate.Execute(SessionName);
			}

			Request->CompleteRequest();
		});
	
	return Request;
}
