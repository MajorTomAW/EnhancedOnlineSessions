// Copyright © 2024 MajorT. All rights reserved.

#include "EnhancedIdentityLibrary.h"

#include "EnhancedOnlineRequests.h"
#include "EnhancedSessionsLibrary.h"


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
