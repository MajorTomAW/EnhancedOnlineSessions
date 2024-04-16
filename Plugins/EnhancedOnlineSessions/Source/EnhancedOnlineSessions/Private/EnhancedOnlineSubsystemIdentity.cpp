// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineSubsystem.h"

#include "EnhancedOnlineRequests.h"
#include "EnhancedOnlineSessions.h"
#include "Kismet/GameplayStatics.h"

void UEnhancedOnlineSubsystem::BindIdentityDelegates(IOnlineIdentityPtr Identity)
{
	check(Identity.IsValid());
}


void UEnhancedOnlineSubsystem::LoginOnlineUser(UEnhancedOnlineRequest_LoginUser* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a bad request."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Request->GetWorld(), Request->LocalUserIndex);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User was called with a bad local player index."));
		Request->OnRequestFailed.Broadcast(TEXT("Login Online User was called with a bad local player index."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("Login Online User is unable to get the local player of the local user index: %d"), Request->LocalUserIndex);
		Request->OnRequestFailed.Broadcast(TEXT("Login Online User is unable to get the local player of the local user index."));
		return;
	}

	LoginOnlineUserInternal(LocalPlayer, Request);
}

void UEnhancedOnlineSubsystem::LoginOnlineUserInternal(ULocalPlayer* LocalPlayer, UEnhancedOnlineRequest_LoginUser* Request)
{
	check(Request->OnlineSub);
	check(Request->Identity);

	ELoginStatus::Type LoginStatus = Request->Identity->GetLoginStatus(LocalPlayer->GetControllerId());
	if (LoginStatus == ELoginStatus::LoggedIn)
	{
		UE_LOG(LogEnhancedSubsystem, Error, TEXT("User is already logged in: [%s]"), *Request->OnlineSub->GetSubsystemName().ToString());
		Request->OnRequestFailed.Broadcast(FString::Printf(TEXT("User is already logged in: [%s]"), *Request->OnlineSub->GetSubsystemName().ToString()));
		return;
	}

	// Convert the AuthType to a string value
	EEnhancedAuthType AuthType = Request->AuthType;
	FString AuthTypeString;
	StaticEnum<EEnhancedAuthType>()->FindNameStringByValue(AuthTypeString, static_cast<int64>(AuthType));

	// Credentials
	FOnlineAccountCredentials Credentials;
	Credentials.Type = AuthTypeString;
	Credentials.Id = Request->UserId;
	Credentials.Token = Request->UserToken;

	Request->Identity->Login(Request->LocalUserIndex, Credentials);
}
