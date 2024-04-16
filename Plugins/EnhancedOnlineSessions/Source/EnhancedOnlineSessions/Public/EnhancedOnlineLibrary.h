// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedOnlineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnhancedOnlineLibrary.generated.h"

class UEnhancedOnlineRequest;
class UEnhancedOnlineRequest_LoginUser;
DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnRequestFailedWithLogin, const FString&, ErrorMessage);

DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnLoginUserSuccess, int32, LocalPlayerIndex);

/**
 * Library that stores all the necessary functions for interacting with online sessions.
 */
UCLASS()
class ENHANCEDONLINESESSIONS_API UEnhancedOnlineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:
	static void SetupFailureDelegate(UEnhancedOnlineRequest* Request, FBlueprintOnRequestFailedWithLogin OnFailed);

public:
	/**
	 * Creates a login user request object that can be used to log in to an online service.
	 * 
	 * @param WorldContextObject		The world context object
	 * @param UserID					The user id that will be used to log in
	 * @param UserToken					The user token, associated with the user id
	 * @param AuthType					The type of authentication to use
	 * @param LocalUserIndex			The index of the local user that is making the request
	 * @param bInvalidateAfterComplete	Should the request be invalidated after it is completed
	 * @param OnFailed					The delegate that will be called if the request fails
	 * @param OnSuccess					The delegate that will be called if the request succeeds
	 * @return The login user request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Identity", meta = (WorldContext = "WorldContextObject", Keywords = "Create, Make, New", AdvancedDisplay = "bInvalidateAfterComplete"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_LoginUser* ConstructOnlineLoginUserRequest(
		UObject* WorldContextObject,
		const FString UserID,
		const FString UserToken,
		const EEnhancedAuthType AuthType,
		int32 LocalUserIndex,
		bool bInvalidateAfterComplete,
		FBlueprintOnLoginUserSuccess OnSuccess,
		FBlueprintOnRequestFailedWithLogin OnFailed);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta = (WorldContext = "WorldContextObject", Keywords = "Create, Make, New", AdvancedDisplay = "bInvalidateAfterComplete", bInvalidatesAfterComplete = "true"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_CreateSession* ConstructOnlineCreateSessionRequest(
		UObject* WorldContextObject,
		const EEnhancedSessionOnlineMode OnlineMode,
		int32 MaxPlayerCount,
		UPARAM(meta = (AllowedTypes = "Map")) FPrimaryAssetId MapId,
		bool bUseLobbiesIfAvailable,
		FString AdvertisementGameModeName,
		int32 LocalUserIndex,
		bool bInvalidateAfterComplete,
		FBlueprintOnRequestFailedWithLogin OnFailed);
};
