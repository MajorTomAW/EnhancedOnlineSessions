// Copyright © 2024 MajorT. All rights reserved.

#include "Libraries/EnhancedIdentityLibrary.h"

#include "EnhancedOnlineRequests.h"
#include "Libraries/EnhancedSessionsLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"

void UEnhancedIdentityLibrary::K2_AddAdmin(APlayerController* InPlayerController)
{
	UGameplayStatics::GetGameMode(InPlayerController->GetWorld())->GameSession->AddAdmin(InPlayerController);
}

void UEnhancedIdentityLibrary::K2_RemoveAdmin(APlayerController* InPlayerController)
{
	UGameplayStatics::GetGameMode(InPlayerController->GetWorld())->GameSession->RemoveAdmin(InPlayerController);
}

bool UEnhancedIdentityLibrary::K2_KickPlayer(APlayerController* InPlayerController, const FText KickReason)
{
	return UGameplayStatics::GetGameMode(InPlayerController->GetWorld())->GameSession->KickPlayer(InPlayerController, KickReason);
}

bool UEnhancedIdentityLibrary::K2_BanPlayer(APlayerController* InPlayerController, const FText BanReason)
{
	return UGameplayStatics::GetGameMode(InPlayerController->GetWorld())->GameSession->BanPlayer(InPlayerController, BanReason);
}

void UEnhancedIdentityLibrary::K2_ChangePlayerName(APlayerController* InPlayerController, const FString NewName)
{
	UGameplayStatics::GetGameMode(InPlayerController->GetWorld())->ChangeName(InPlayerController, NewName, true);
}

UEnhancedOnlineRequest_LoginUser* UEnhancedIdentityLibrary::ConstructOnlineLoginUserRequest(UObject* WorldContextObject,
                                                                                            const EEnhancedLoginAuthType AuthType, const FString UserId, const FString AuthToken,
                                                                                            const int32 LocalUserIndex, const bool bInvalidateOnCompletion, FBPOnLoginRequestSuceeded OnSucceededDelegate,
                                                                                            FBPOnRequestFailedWithLog OnFailedDelegate)
{
	UEnhancedOnlineRequest_LoginUser* Request = NewObject<UEnhancedOnlineRequest_LoginUser>(WorldContextObject);
	Request->ConstructRequest();

	Request->AuthType = AuthType;
	Request->UserId = UserId;
	Request->AuthToken = AuthToken;
	Request->LocalUserIndex = LocalUserIndex;
	Request->bInvalidateOnCompletion = bInvalidateOnCompletion;

	UEnhancedSessionsLibrary::SetupFailureDelegate(Request, OnFailedDelegate);

	Request->OnUserLoginCompleted.AddLambda(
		[OnSucceededDelegate, Request] (int32 LocalUserIndex)
		{
			if (OnSucceededDelegate.IsBound())
			{
				OnSucceededDelegate.Execute(LocalUserIndex);
			}

			Request->CompleteRequest();
		});

	return Request;
}
