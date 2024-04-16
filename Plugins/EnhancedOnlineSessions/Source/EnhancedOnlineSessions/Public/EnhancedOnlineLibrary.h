// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedOnlineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnhancedOnlineLibrary.generated.h"

class UEnhancedOnlineRequest;
class UEnhancedOnlineRequest_CreateSession;
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
	 * @param WorldContextObject		he world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param UserID					The user id that will be used to log in
	 * @param UserToken					The user token, associated with the user id
	 * @param AuthType					The type of authentication to use
	 * @param LocalUserIndex			The index of the local user that is making the request
	 * @param bInvalidateAfterComplete	Should the request be invalidated after it is completed, default to true to save memory
	 * @param OnFailed					The delegate that will be called if the request fails
	 * @param OnSuccess					The delegate that will be called if the request succeeds
	 * @return The login user request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Identity", meta =
		(WorldContext = "WorldContextObject", Keywords = "Create, Make, New", AdvancedDisplay = "bInvalidateAfterComplete", bInvalidatesAfterComplete = "true"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_LoginUser* ConstructOnlineLoginUserRequest(
		UObject* WorldContextObject,
		const FString UserID,
		const FString UserToken,
		const EEnhancedAuthType AuthType,
		int32 LocalUserIndex,
		bool bInvalidateAfterComplete,
		FBlueprintOnLoginUserSuccess OnSuccess,
		FBlueprintOnRequestFailedWithLogin OnFailed);

	/**
	 * Creates a new session with the given settings.
	 * 
	 * @param WorldContextObject			The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param OnlineMode					The online mode of the session
	 * @param MaxPlayerCount				The maximum number of players that can join the session
	 * @param MapId							The id of the map that will be used for the session
	 * @param SessionFriendlyName			The friendly name of the session, only for display purposes
	 * @param SearchKeyword					The keyword that will be used to search for the session
	 * @param bUseLobbiesIfAvailable		Should lobbies be used if the online service supports them
	 * @param bUseVoiceChatIfAvailable		Should voice chat be used in the session if the online service supports it
	 * @param bUseServerTravelOnSuccess		Should the server travel to the session map. default should be true
	 * @param AdvertisementGameModeName		The name of the game mode that will be advertised, only for display purposes or matchmaking
	 * @param LocalUserIndex				The index of the local user that is making the request
	 * @param bInvalidateAfterComplete		Should the request be invalidated after it is completed, default to true to save memory
	 * @param OnFailed						The delegate that will be called if the request fails
	 * @return The create session request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Create, Make, New",
			AdvancedDisplay = "bInvalidateAfterComplete",
			OnlineMode = "Online", MaxPlayerCount = "4", SearchKeyword = "default", AdvertisementGameModeName = "default", bUseLobbiesIfAvailable = "true",
			bUseVoiceChatIfAvailable = "false", bUseServerTravelOnSuccess = "true", bInvalidatesAfterComplete = "true"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_CreateSession* ConstructOnlineCreateSessionRequest(
		UObject* WorldContextObject,
		const EEnhancedSessionOnlineMode OnlineMode,
		int32 MaxPlayerCount,
		UPARAM(meta = (AllowedTypes = "Map")) FPrimaryAssetId MapId,
		FString SessionFriendlyName,
		FString SearchKeyword,
		bool bUseLobbiesIfAvailable,
		bool bUseVoiceChatIfAvailable,
		bool bUseServerTravelOnSuccess,
		FString AdvertisementGameModeName,
		int32 LocalUserIndex,
		bool bInvalidateAfterComplete,
		FBlueprintOnRequestFailedWithLogin OnFailed);
};
