// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedOnlineRequests.h"
#include "EnhancedOnlineSessionsSubsystem.h"
#include "EnhancedOnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlinePresenceInterface.h"

void UEnhancedOnlineSessionsSubsystem::GetFriendsList(UEnhancedOnlineRequest_GetFriendsList* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List was called with a bad request."));
		return;
	}

	if (IsValid(PendingGetFriendsListRequest))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("A get friends list request is already pending."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("A get friends list request is already pending."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Get Friends List was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List was called with a bad local user index: %d."), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Get Friends List was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	GetFriendsListInternal(LocalPlayer, Request);
}

void UEnhancedOnlineSessionsSubsystem::GetFriendsListInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_GetFriendsList* Request)
{
	EFriendsLists::Type FriendsListType = EFriendsLists::Default;

	if (Request->bFilterOnlyOnlineFriends)
	{
		FriendsListType = EFriendsLists::OnlinePlayers;
	}

	if (Request->bFilterOnlyInGameFriends)
	{
		FriendsListType = EFriendsLists::InGamePlayers;
	}
	
	PendingGetFriendsListRequest = Request;
	const bool bCalled = Request->Friends->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString(FriendsListType),
		FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::HandleGetFriendsListComplete));

	if (!bCalled)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List failed to call the read friends list function."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Get Friends List failed to call the read friends list function."));
		PendingGetFriendsListRequest = nullptr;
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleGetFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString)
{
	if (PendingGetFriendsListRequest == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List completed with no pending request."));
		return;
	}
	
	if (bWasSuccessful)
	{
		TArray<FEnhancedBlueprintFriendInfo> BPFoundFriends;
		TArray<TSharedRef<FOnlineFriend>> FoundFriends;
		PendingGetFriendsListRequest->Friends->GetFriendsList(LocalUserNum, ListName, FoundFriends);

		for (auto& Friend : FoundFriends)
		{
			FEnhancedBlueprintFriendInfo BPFriend;
			BPFriend.DisplayName = Friend->GetDisplayName();
			BPFriend.RealName = Friend->GetRealName();
			BPFriend.UniqueNetId = Friend->GetUserId();
			BPFriend.PresenceInfo.bIsJoinable = Friend->GetPresence().bIsJoinable;
			BPFriend.PresenceInfo.bIsOnline = Friend->GetPresence().bIsOnline;
			BPFriend.PresenceInfo.bIsPlaying = Friend->GetPresence().bIsPlaying;
			BPFriend.PresenceInfo.bIsPlayingThisGame = Friend->GetPresence().bIsPlayingThisGame;
			BPFriend.PresenceInfo.bHasVoiceSupport = Friend->GetPresence().bHasVoiceSupport;
			BPFriend.PresenceInfo.PresenceState = static_cast<EBlueprintEnhancedPresenceState>(static_cast<int32>(Friend->GetPresence().Status.State));
			BPFriend.PresenceInfo.StatusString = Friend->GetPresence().Status.StatusStr;

			BPFoundFriends.Add(BPFriend);
		}

		PendingGetFriendsListRequest->OnGetFriendsListCompleted.Broadcast(BPFoundFriends);
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Get Friends List failed: %s"), *ErrorString);
		PendingGetFriendsListRequest->OnRequestFailedDelegate.Broadcast(ErrorString);
	}

	PendingGetFriendsListRequest->CompleteRequest();
	PendingGetFriendsListRequest = nullptr;
}


void UEnhancedOnlineSessionsSubsystem::FindFriendOnlineSessions(UEnhancedOnlineRequest_FindFriendSession* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions was called with a bad request."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Find Friend Online Sessions was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions was called with a bad local user index: %d. Looks like the online service does not support that action?"), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Find Friend Online Sessions was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	FindFriendOnlineSessionsInternal(LocalPlayer, Request);
}

void UEnhancedOnlineSessionsSubsystem::FindFriendOnlineSessionsInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_FindFriendSession* Request)
{
	check(Request->Sessions);

	FindFriendSessionsDelegateHandle = Request->Sessions->AddOnFindFriendSessionCompleteDelegate_Handle(LocalPlayer->GetControllerId(), FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindFriendOnlineSessionsComplete));
	PendingFindFriendSessionRequest = Request;

	const bool bCalled = Request->Sessions->FindFriendSession(LocalPlayer->GetControllerId(), *Request->FriendId.GetUniqueNetId());

	if (!bCalled)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions failed to call the find friend session function: NetId: %s, LocalUserIndex: %d. The used online service may not support this."), *Request->FriendId.GetUniqueNetId()->ToString(), LocalPlayer->GetControllerId());
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Find Friend Online Sessions failed to call the find friend session function."));
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleFindFriendOnlineSessionsComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionInfo)
{
	if (PendingFindFriendSessionRequest == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions completed with no pending request."));
		return;
	}

	if (bWasSuccessful)
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
				
			UE_LOG(LogEnhancedSubsystem, Log, TEXT("\tFound Friend session (UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
			*OwningUserId,
			*SearchResult.Session.OwningUserName,
			SearchResult.Session.NumOpenPrivateConnections,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.PingInMs);
		}
		
		PendingFindFriendSessionRequest->OnFindFriendSessionCompleted.Broadcast(Results);
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Find Friend Online Sessions failed: %i"), SessionInfo.Num());
		PendingFindFriendSessionRequest->OnRequestFailedDelegate.Broadcast(TEXT("Find Friend Online Sessions failed."));
	}
}