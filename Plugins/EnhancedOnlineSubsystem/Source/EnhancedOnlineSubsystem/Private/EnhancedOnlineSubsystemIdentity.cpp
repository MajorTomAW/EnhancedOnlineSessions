// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedOnlineRequests.h"
#include "EnhancedOnlineSessionsSubsystem.h"
#include "EnhancedOnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UEnhancedOnlineSessionsSubsystem::LoginOnlineUser(UEnhancedOnlineRequest_LoginUser* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a bad request."));
		return;
	}

	if (IsValid(PendingLoginRequest))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("A login request is already pending."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("A login request is already pending."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Login Online User was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a bad local user index: %d."), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Login Online User was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	LoginOnlineUserInternal(LocalPlayer, Request);
}

void UEnhancedOnlineSessionsSubsystem::LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LoginUser* Request)
{
	if (Request->Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a user that is already logged in."));
		Request->OnUserLoginCompleted.Broadcast(0);
		return;
	}

	LoginDelegateHandle = Request->Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &UEnhancedOnlineSessionsSubsystem::HandleLoginComplete));
	PendingLoginRequest = Request;

	FString AuthTypeString;
	StaticEnum<EEnhancedLoginAuthType>()->FindNameStringByValue(AuthTypeString, static_cast<int32>(Request->AuthType));
	AuthTypeString = AuthTypeString.ToLower();
		
	FOnlineAccountCredentials Credentials;
	Credentials.Type = AuthTypeString;
	Credentials.Token = Request->AuthToken;
	Credentials.Id = Request->UserId;

	UE_LOG(LogEnhancedSubsystem, Log, TEXT("Logging in user with type: %s, token: %s, id: %s"), *Credentials.Type, *Credentials.Token, *Credentials.Id);

	if (!Request->Identity->Login(0, Credentials))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User failed."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Login Online User failed."));
		Request->InvalidateRequest();
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();

	if (bWasSuccessful)
	{
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Login Online User succeeded."));

		if (PendingLoginRequest)
		{
			PendingLoginRequest->OnUserLoginCompleted.Broadcast(LocalUserNum);
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("We lost the pending login request?? D:"))
		}
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User failed: %s"), *Error);

		if (PendingLoginRequest)
		{
			PendingLoginRequest->OnRequestFailedDelegate.Broadcast(Error);
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("We lost the pending login request?? D:"))
		}
	}

	Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
	LoginDelegateHandle.Reset();

	if (PendingSessionRequest)
	{
		PendingLoginRequest->CompleteRequest();
	}

	PendingSessionRequest = nullptr;
}

void UEnhancedOnlineSessionsSubsystem::LogoutOnlineUser(UEnhancedOnlineRequest_LogoutUser* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User was called with a bad request."));
		return;
	}

	if (IsValid(PendingLoginRequest))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("A logout request is already pending."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("A logout request is already pending."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User was called with a bad local user index."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Logout Online User was called with a bad local user index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User was called with a bad local user index: %d."), Request->LocalUserIndex);
		Request->OnRequestFailedDelegate.Broadcast(FString::Printf(TEXT("Logout Online User was called with a bad local user index: %d."), Request->LocalUserIndex));
		return;
	}

	LogoutOnlineUserInternal(LocalPlayer, Request);
}

void UEnhancedOnlineSessionsSubsystem::LogoutOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LogoutUser* Request)
{
	if (Request->Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User was called with a user that is already logged out."));
		Request->OnUserLogoutCompleted.Broadcast(LocalPlayer->GetControllerId());
		return;
	}

	LogoutDelegateHandle = Request->Identity->AddOnLogoutCompleteDelegate_Handle(0, FOnLogoutCompleteDelegate::CreateUObject(this, &UEnhancedOnlineSessionsSubsystem::HandleLogoutComplete));
	PendingLogoutRequest = Request;

	UE_LOG(LogEnhancedSubsystem, Log, TEXT("Logging out user."));

	if (!Request->Identity->Logout(0))
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User failed."));
		Request->OnRequestFailedDelegate.Broadcast(TEXT("Logout Online User failed."));
		Request->InvalidateRequest();
	}
}

void UEnhancedOnlineSessionsSubsystem::HandleLogoutComplete(int32 LocalUserNum, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();

	if (bWasSuccessful)
	{
		UE_LOG(LogEnhancedSubsystem, Log, TEXT("Logout Online User succeeded."));

		if (PendingLogoutRequest)
		{
			PendingLogoutRequest->OnUserLogoutCompleted.Broadcast(LocalUserNum);
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("We lost the pending logout request?? D:"))
		}
	}
	else
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Logout Online User failed"));

		if (PendingLogoutRequest)
		{
			PendingLogoutRequest->OnRequestFailedDelegate.Broadcast(TEXT("Logout Online User failed."));
		}
		else
		{
			UE_LOG(LogEnhancedSubsystem, Error, TEXT("We lost the pending login request?? D:"))
		}
	}

	Identity->ClearOnLogoutCompleteDelegate_Handle(LocalUserNum, LogoutDelegateHandle);
	LogoutDelegateHandle.Reset();

	if (PendingLogoutRequest)
	{
		PendingLogoutRequest->CompleteRequest();
	}

	PendingLogoutRequest = nullptr;
}

