// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonOnlineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonOnlineSessionsLibrary.generated.h"

class UCommonOnlineSearchResult;
class UCommonOnline_FindSessionsRequest;
class UCommonOnlineRequest;
struct FStoredSessionSettings;
enum class ECommonSessionOnlineMode : uint8;
class UCommonOnline_CreateSessionRequest;
enum class EAuthType : uint8;
class UCommonOnline_LoginUserRequest;

DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnRequestFailedWithLog, const FString&, Log);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnLoginSuccess, int32, LocalPlayerIndex);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBlueprintOnOnFindSessionsSuccess, const TArray<UCommonOnlineSearchResult*>&, SearchResults);


/**
 * Library that stores all the necessary functions for interacting with online sessions.
 */
UCLASS()
class COMMONONLINESESSIONS_API UCommonOnlineSessionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Creates a login user request object that can be used to log in to an online service.
	 * 
	 * @param WorldContextObject		The world context object
	 * @param UserID					The user id that will be used to log in
	 * @param UserToken					The user token, associated with the user id
	 * @param AuthType					The type of authentication to use
	 * @param OnFailed					The delegate that will be called if the request fails
	 * @param OnSuccess					The delegate that will be called if the request succeeds
	 * @return The login user request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|Common Sessions|Identity", meta = (WorldContext = "WorldContextObject", Keywords = "Create, Make, New"))
	static UPARAM(DisplayName = "Request") UCommonOnline_LoginUserRequest* ConstructOnlineLoginUserRequest(UObject* WorldContextObject, const FString UserID, const FString UserToken, const EAuthType AuthType, FBlueprintOnLoginSuccess OnSuccess, FBlueprintOnRequestFailedWithLog OnFailed);

	/**
	 * Creates a create session request object that can be used to create a new session.
	 * 
	 * @param WorldContextObject		The world context object
	 * @param MaxPlayerCount			The maximum number of players that can join the session
	 * @param MapID						The map id that will be used for the session
	 * @param OnlineMode				The online mode that will be used for the session
	 * @param SessionFriendlyName		The friendly name of the session
	 * @param SearchKeyword				The search keyword that will be used to find the session
	 * @param bUseLobbiesIfAvailable	Whether to use lobbies if available
	 * @param bUseVoiceChatIfAvailable	Whether to use voice chat if available
	 * @param StoredSettings			The stored settings that will be used for the session
	 * @param OnFailed					The delegate that will be called if the request fails
	 * @return The create session request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|Common Sessions|Sessions", meta = (WorldContext = "WorldContextObject", Keywords = "Create, Make, New", AdvancedDisplay = "SearchKeyword, bEnabledVoiceChatIfAvailable, StoredSettings", AutoCreateRefTerm = "StoredSettings", MaxPlayerCount = 10, MapID  = "None", OnlineMode = "Online", SessionFriendlyName = "", GameModeFriendlyName = "", SearchKeyword = "", bUseLobbiesIfAvailable = "true", bUseVoiceChatIfAvailable = "false", StoredSettings = "None"))
	static UPARAM(DisplayName = "Request") UCommonOnline_CreateSessionRequest* ConstructOnlineCreateSessionRequest(UObject* WorldContextObject, int32 MaxPlayerCount, UPARAM(meta = (AllowedTypes = "Map")) FPrimaryAssetId MapID, ECommonSessionOnlineMode OnlineMode, FString SessionFriendlyName, FString GameModeFriendlyName, FString SearchKeyword, bool bUseLobbiesIfAvailable, bool bUseVoiceChatIfAvailable, FStoredSessionSettings StoredSettings, FBlueprintOnRequestFailedWithLog OnFailed);

	/**
	 * Invalidates the given request object.
	 * 
	 * @param Request	The request object to invalidate
	 * @return True if the request was successfully invalidated, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|Common Sessions|Requests")
	static bool InvalidateRequest(UCommonOnlineRequest* Request);

	/**
	 * Creates a find sessions request object that can be used to search for sessions.
	 * 
	 * @param WorldContextObject		The world context object
	 * @param MaxNumResults				The maximum number of results to return
	 * @param OnlineMode				The online mode that will be used for the session
	 * @param bSearchLobbies			Whether to search for lobbies
	 * @param OnSuccess					The delegate that will be called if the request succeeds
	 * @param OnFailed					The delegate that will be called if the request fails
	 * @return The find sessions request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|Common Sessions|Sessions", meta = (WorldContext = "WorldContextObject", Keywords = "Create, Make, New", MaxNumResults = -1, OnlineMode = "Online", bSearchLobbies = "true"))
	static UPARAM(DisplayName = "Request") UCommonOnline_FindSessionsRequest* ConstructOnlineFindSessionsRequest(UObject* WorldContextObject, int32 MaxNumResults, ECommonSessionOnlineMode OnlineMode, bool bSearchLobbies, FBlueprintOnOnFindSessionsSuccess OnSuccess, FBlueprintOnRequestFailedWithLog OnFailed);
};
