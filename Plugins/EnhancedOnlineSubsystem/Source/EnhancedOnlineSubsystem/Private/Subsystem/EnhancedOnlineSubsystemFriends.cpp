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