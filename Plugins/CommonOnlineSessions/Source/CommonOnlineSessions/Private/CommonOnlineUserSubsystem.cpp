// Copyright © 2024 MajorT. All rights reserved.

#include "CommonOnlineSubsystem.h"

#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AssetRegistry/AssetData.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"

void UCommonOnlineSubsystem::BindOnlineIdentityDelegates(IOnlineSubsystem* OnlineSub)
{
	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());
	
	// Bind to the login complete delegate for all local players
	for (int32 PlayerIndex = 0; PlayerIndex < MAX_LOCAL_PLAYERS; PlayerIndex++)
	{
		Identity->AddOnLoginCompleteDelegate_Handle(PlayerIndex, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));
		Identity->AddOnLoginStatusChangedDelegate_Handle(PlayerIndex, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::OnLoginStatusChanged));
	}

	const IOnlineFriendsPtr Friends = OnlineSub->GetFriendsInterface();
	if (Friends)
	{
		//Friends->AddOn
	}
}

void UCommonOnlineSubsystem::RemoveOnlineIdentityDelegates(IOnlineSubsystem* OnlineSub)
{
	const IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	if (Identity)
	{
		Identity->ClearOnLoginChangedDelegates(this);

		for (int32 PlayerIndex = 0; PlayerIndex < MAX_LOCAL_PLAYERS; PlayerIndex ++)
		{
			Identity->ClearOnLoginCompleteDelegates(PlayerIndex, this);
			Identity->ClearOnLoginStatusChangedDelegates(PlayerIndex, this);
			Identity->ClearOnLogoutCompleteDelegates(PlayerIndex, this);	
		}
	}
}



#pragma region login_online_user
void UCommonOnlineSubsystem::LoginOnlineUser(APlayerController* PlayerToLogin, UCommonOnline_LoginUserRequest* LoginRequest)
{
	// Make sure the request is valid
	if (LoginRequest == nullptr)
	{
		LoginRequest->OnFailed.Broadcast(TEXT("Login Online User was called with a bad request"));
		return;
	}

	// Get the local player and return if it's invalid
	ULocalPlayer* LocalPlayer = (PlayerToLogin != nullptr) ? PlayerToLogin->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		LoginRequest->OnFailed.Broadcast(TEXT("Unable to find local player"));
		return;
	}

	LoginOnlineUserInternal(LocalPlayer, LoginRequest);
}

void UCommonOnlineSubsystem::LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LoginUserRequest* LoginRequest)
{
	const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub)

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	ELoginStatus::Type LoginStatus = Identity->GetLoginStatus(LocalPlayer->GetControllerId());
	if (LoginStatus == ELoginStatus::LoggedIn)
	{
		LoginRequest->OnFailed.Broadcast(FString::Printf(TEXT("User is already logged in: [%s]"), *OnlineSub->GetSubsystemName().ToString()));
		return;
	}

	EAuthType AuthType = LoginRequest->AuthType;
	FString AuthTypeString;
	StaticEnum<EAuthType>()->FindNameStringByValue(AuthTypeString, static_cast<int64>(AuthType));

	// Create the credentials
	FOnlineAccountCredentials Credentials;
	Credentials.Type = AuthTypeString;
	Credentials.Id = LoginRequest->UserId;
	Credentials.Token = LoginRequest->UserToken;

	// Bind to the delegate which lives on the LoginRequest
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
	FOnlineResultInfo ResultInfo;
	if (!bWasSuccessful)
	{
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("Login Failed!");
		ResultInfo.ErrorMessage = Error;
	}

	NotifyUserLoginComplete(LocalUserNum, UserId, ResultInfo);
}

void UCommonOnlineSubsystem::NotifyUserLoginComplete(int32 LocalUserNum, const FUniqueNetId& UserId, const FOnlineResultInfo& ResultInfo)
{
}

void UCommonOnlineSubsystem::LogoutOnlineUser(APlayerController* PlayerToLogout, UCommonOnline_LogoutUserRequest* LogoutRequest)
{
	ULocalPlayer* LocalPlayer = (PlayerToLogout != nullptr) ? PlayerToLogout->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogOnline, Error, TEXT("Unable to find local player"));
		return;
	}

	LogoutOnlineUserInternal(LocalPlayer, LogoutRequest);
}

void UCommonOnlineSubsystem::LogoutOnlineUserInternal(ULocalPlayer* LocalPlayer, UCommonOnline_LogoutUserRequest* LogoutRequest)
{
	const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub)

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity.IsValid());

	// Bind to the delegates which live on the LogoutRequest
	Identity->OnLogoutCompleteDelegates->AddLambda(
		[this, LogoutRequest] (int32 LocalUserNum, bool bWasSuccessful)
		{
			if (bWasSuccessful)
			{
				LogoutRequest->OnLogoutUserSuccess.Broadcast(LocalUserNum);
			}
			else
			{
				LogoutRequest->OnFailed.Broadcast(TEXT("Failed to logout, user already logged out"));
			}
		});

	Identity->Logout(LocalPlayer->GetControllerId());
}

void UCommonOnlineSubsystem::OnLoginStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId)
{
}

void UCommonOnlineSubsystem::NotifyUserLoginStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId)
{
}
#pragma endregion

void UCommonOnlineSubsystem::GetOnlineFriendsList(APlayerController* PlayerRequesting, UCommonOnline_GetFriendsListRequest* FriendsRequest)
{
	if (FriendsRequest == nullptr)
	{
		UE_LOG(LogOnline, Error, TEXT("GetOnlineFriendsList was called with a bad request"));
		return;
	}

	if (PlayerRequesting == nullptr)
	{
		UE_LOG(LogOnline, Error, TEXT("GetOnlineFriendsList was called with a bad player"));
		FriendsRequest->OnFailed.Broadcast(TEXT("GetOnlineFriendsList was called with a bad player"));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerRequesting->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogOnline, Error, TEXT("GetOnlineFriendsList was called with a bad local player"));
		FriendsRequest->OnFailed.Broadcast(TEXT("GetOnlineFriendsList was called with a bad local player"));
		return;
	}

	GetOnlineFriendsListInternal(LocalPlayer, FriendsRequest);
}

void UCommonOnlineSubsystem::GetOnlineFriendsListInternal(ULocalPlayer* LocalPlayer, UCommonOnline_GetFriendsListRequest* FriendsRequest)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineFriendsPtr Friends = OnlineSub->GetFriendsInterface();
	if (!Friends.IsValid())
	{
		FriendsRequest->OnFailed.Broadcast(FString::Printf(TEXT("Friends not supported by the online subsystem: [%s]"), *OnlineSub->GetSubsystemName().ToString()));
		return;
	}

	PendingFriendsListRequest = FriendsRequest;
	Friends->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default), FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnGetFriendsListComplete));
}

void UCommonOnlineSubsystem::OnGetFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString)
{
	if (PendingFriendsListRequest == nullptr)
	{
		UE_LOG(LogOnline, Error, TEXT("Received a friends list response with no pending request"));
		return;
	}

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineFriendsPtr Friends = OnlineSub->GetFriendsInterface();
	if (!Friends.IsValid())
	{
		PendingFriendsListRequest->OnFailed.Broadcast(FString::Printf(TEXT("Friends not supported by the online subsystem: [%s]"), *OnlineSub->GetSubsystemName().ToString()));
		return;
	}

	if (bWasSuccessful)
	{
		TArray<FCommonOnlineFriendInfo> FoundFriends;
		TArray<TSharedRef<FOnlineFriend>> FriendsList;
		Friends->GetFriendsList(LocalUserNum, ListName, FriendsList);
		
		ECommonFriendOnlineSateFilter OnlineFilter = PendingFriendsListRequest->OnlineFilter;

		for (const TSharedRef<FOnlineFriend>& Friend : FriendsList)
		{
			FCommonOnlineFriendInfo FriendInfo;
			
			FOnlineUserPresence Presence = Friend->GetPresence();
			if (OnlineFilter != ECommonFriendOnlineSateFilter::All)
			{
				ECommonFriendOnlineSate OnlineState = static_cast<ECommonFriendOnlineSate>(Presence.Status.State);

				if (OnlineState != static_cast<ECommonFriendOnlineSate>(OnlineFilter))
				{
					continue;
				}
			}

			FriendInfo.DisplayName = Friend->GetDisplayName();
			FriendInfo.RealName = Friend->GetRealName();
			FriendInfo.OnlineState = static_cast<ECommonFriendOnlineSate>(Presence.Status.State);

			FriendInfo.PresenceInfo.bIsJoinable = Presence.bIsJoinable;
			FriendInfo.PresenceInfo.bHasVoiceSupport = Presence.bHasVoiceSupport;
			FriendInfo.PresenceInfo.StatusString = Presence.Status.StatusStr;
			FriendInfo.PresenceInfo.bIsOnline = Presence.bIsOnline;
			FriendInfo.PresenceInfo.bIsPlaying = Presence.bIsPlaying;
			FriendInfo.PresenceInfo.bIsPlayingThisGame = Presence.bIsPlayingThisGame;

			FoundFriends.Add(FriendInfo);
		}

		PendingFriendsListRequest->OnGetFriendsListComplete.Broadcast(FoundFriends);
	}
	else
	{
		PendingFriendsListRequest->OnFailed.Broadcast(ErrorString);
	}

	PendingFriendsListRequest = nullptr;
}

