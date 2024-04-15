// Copyright © 2024 MajorT. All rights reserved.


#include "CommonOnlineTypes.h"

void FOnlineResultInfo::FromOnlineError(const FOnlineError& OnlineError)
{
	bWasSuccessful = OnlineError.WasSuccessful();
	ErrorId = OnlineError.GetErrorCode();
	ErrorMessage = OnlineError.GetErrorMessage().ToString();
}
