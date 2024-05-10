// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnhancedSessionsLibrary.generated.h"

class UEnhancedOnlineRequest_StartSession;
class UEnhancedOnlineRequest_JoinSession;
class UEnhancedOnlineRequest_FindSessions;
class UEnhancedSessionSearchResult;
enum class EEnhancedLoginAuthType : uint8;
class UEnhancedOnlineRequest_LoginUser;
class UEnhancedOnlineRequest_CreateLobby;
class UEnhancedOnlineRequestBase;
enum class EEnhancedSessionOnlineMode : uint8;
class UEnhancedOnlineRequest_CreateSession;

/**
 * Delegate for when a request fails
 * @param Reason	The reason why the request failed
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FBPOnRequestFailedWithLog, const FString&, Reason);

/**
 * Delegate for when a host lobby request succeeds
 * @param SessionName		The name of the session
 * @param LocalUserIndex	The index of the local user who made the request
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBPOnHostLobbyRequestSucceeded, const FName&, SessionName, const int32&, LocalUserIndex);

/**
 * Delegate for when starting a session request succeeds
 * @param SessionName	The name of the session
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FBPOnStartSessionRequestSucceeded, const FName&, SessionName);

/**
 * Delegate for when a find sessions request succeeds
 * @param SearchResults	List of found sessions
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FBPOnFindSessionsSuceeeded, const TArray<UEnhancedSessionSearchResult*>&, SearchResults);

/**
 * Library of functions for interacting with the Enhanced Online Subsystem
 */
UCLASS()
class ENHANCEDONLINESUBSYSTEM_API UEnhancedSessionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void SetupFailureDelegate(UEnhancedOnlineRequestBase* Request, FBPOnRequestFailedWithLog OnFailedDelegate);

public:
	/**
	 * Gets the ping of a search result in milliseconds
	 * @param SearchResult	The search result to get the ping of
	 * @return The ping in milliseconds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions")
	static int32 GetPingInMs(UEnhancedSessionSearchResult* SearchResult);

	/**
	 * Gets the number of players in a search result
	 * @param SearchResult	The search result to get the number of players of
	 * @return The number of players
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions")
	static int32 GetMaxPlayers(UEnhancedSessionSearchResult* SearchResult);

	/**
	 * Gets the number of players in a search result
	 * @param SearchResult	The search result to get the number of players of
	 * @return The number of players
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions")
	static int32 GetCurrentPlayers(UEnhancedSessionSearchResult* SearchResult);

	/**
	 * Gets the session ID of a search result
	 * @param SearchResult	The search result to get the session ID of
	 * @return The session ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions")
	static FString GetSessionFriendlyName(UEnhancedSessionSearchResult* SearchResult);

public:
	/**
	 * Constructs a request to create an online session
	 * @param WorldContextObject			The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param OnlineMode					The online mode of the session
	 * @param MaxPlayerCount				The maximum number of players allowed in the session
	 * @param MapId							The map to use for the session
	 * @param TravelURLOperators			The travel URL operators that will be appended to the travel URL
	 * @param FriendlyName					The friendly name of the session, used to display in the UI
	 * @param SearchKeyword					The search keyword for the session, used to find the session. Multiple sessions can share the same keyword
	 * @param bUseLobbiesIfAvailable		Whether to use lobbies if available
	 * @param bUseVoiceChatIfAvailable		Whether to use voice chat if available
	 * @param GameModeAdvertisementName		The name of the game mode to advertise
	 * @param bIsPresence					Whether the session uses presence
	 * @param bAllowJoinInProgress			Whether to allow players to join while the session is in progress
	 * @param LocalUserIndex				The index of the local user who made the request
	 * @param bInvalidateOnCompletion		Whether to invalidate the request when it's completed
	 * @param OnFailedDelegate				Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Make, Create, New",
			AdvancedDisplay = "bInvalidateOnCompletion, LocalUserIndex",
			MaxPlayerCount = "4", SearchKeyword = "default", GameModeAdvertisementName = "default", bInvalidateOnCompletion = "true", LocalUserIndex = "0", bAllowJoinInProgress = "true", bUseLobbiesIfAvailable = "false", bUseVoiceChatIfAvailable = "false", AutoCreateRefTerm = "TravelURLOperators"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_CreateSession* ConstructOnlineHostSessionRequest(
		UObject* WorldContextObject,
		const EEnhancedSessionOnlineMode OnlineMode,
		const int32 MaxPlayerCount,
		UPARAM(meta = (AllowedTypes = "Map")) FPrimaryAssetId MapId,
		UPARAM(meta = (DisplayName = "Travel URL Operators")) TArray<FString> TravelURLOperators,
		const FString FriendlyName,
		const FString SearchKeyword,
		const bool bUseLobbiesIfAvailable,
		const bool bUseVoiceChatIfAvailable,
		const FString GameModeAdvertisementName,
		const bool bIsPresence,
		const bool bAllowJoinInProgress,
		const int32 LocalUserIndex,
		const bool bInvalidateOnCompletion,
		FBPOnRequestFailedWithLog OnFailedDelegate);

	/**
	 * Constructs a request to create an online Peer to Peer lobby
	 * @param WorldContextObject			The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param OnlineMode					The online mode of the session
	 * @param MaxPlayerCount				The maximum number of players allowed in the session
	 * @param FriendlyName					The friendly name of the session, used to display in the UI
	 * @param SearchKeyword					The search keyword for the session, used to find the session. Multiple sessions can share the same keyword
	 * @param bUseVoiceChatIfAvailable		Whether to use voice chat if available
	 * @param GameModeAdvertisementName		The name of the game mode to advertise
	 * @param bIsPresence					Whether the session uses presence
	 * @param bAllowJoinInProgress			Whether to allow players to join while the lobby is in progress
	 * @param LocalUserIndex				The index of the local user who made the request
	 * @param bInvalidateOnCompletion		Whether to invalidate the request when it's completed
	 * @param OnSucceededDelegate			Delegate to call when the request succeeds
	 * @param OnFailedDelegate				Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Make, Create, New", DisplayName = "Construct Online Host P2P Lobby Request",
			AdvancedDisplay = "bInvalidateOnCompletion, LocalUserIndex",
			MaxPlayerCount = "4", SearchKeyword = "default", GameModeAdvertisementName = "default", LocalUserIndex = "0", bAllowJoinInProgress = "false"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_CreateLobby* ConstructOnlineHostLobbyRequest(
		UObject* WorldContextObject,
		const EEnhancedSessionOnlineMode OnlineMode,
		const int32 MaxPlayerCount,
		const FString FriendlyName,
		const FString SearchKeyword,
		const bool bUseVoiceChatIfAvailable,
		const FString GameModeAdvertisementName,
		const bool bIsPresence,
		const bool bAllowJoinInProgress,
		const int32 LocalUserIndex,
		bool bInvalidateOnCompletion,
		FBPOnHostLobbyRequestSucceeded OnSucceededDelegate,
		FBPOnRequestFailedWithLog OnFailedDelegate);

	/**
	 * Constructs a request to find online sessions
	 * @param WorldContextObject	The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param OnlineMode			The online mode of the session
	 * @param MaxSearchResults		The maximum number of search results to return
	 * @param bFindLobbies			Whether to find lobbies
	 * @param SearchKeyword			The search keyword to use
	 * @param LocalUserIndex		The index of the local user who made the request
	 * @param bInvalidateOnCompletion	Whether to invalidate the request when it's completed
	 * @param OnSucceededDelegate	Delegate to call when the request succeeds
	 * @param OnFailedDelegate		Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Make, Create, New", DisplayName = "Construct Online Find Sessions Request",
			AdvancedDisplay = "LocalUserIndex, bInvalidateOnCompletion", LocalUserIndex = "0", bFindLobbies = "true", bInvalidateOnCompletion = "false"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_FindSessions* ConstructOnlineFindSessionsRequest(
		UObject* WorldContextObject,
		const EEnhancedSessionOnlineMode OnlineMode,
		const int32 MaxSearchResults,
		const bool bFindLobbies,
		const FString SearchKeyword,
		const int32 LocalUserIndex,
		const bool bInvalidateOnCompletion,
		FBPOnFindSessionsSuceeeded OnSucceededDelegate,
		FBPOnRequestFailedWithLog OnFailedDelegate);

	/**
	 * Constructs a request to join an online session
	 * @param WorldContextObject	The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param SessionToJoin			The session to join
	 * @param LocalUserIndex		The index of the local user who made the request
	 * @param bInvalidateOnCompletion	Whether to invalidate the request when it's completed
	 * @param OnFailedDelegate		Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Make, Create, New", DisplayName = "Construct Online Join Session Request",
			AdvancedDisplay = "LocalUserIndex, bInvalidateOnCompletion", LocalUserIndex = "0", bInvalidateOnCompletion = "true"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_JoinSession* ConstructOnlineJoinSessionRequest(
		UObject* WorldContextObject,
		UEnhancedSessionSearchResult* SessionToJoin,
		const int32 LocalUserIndex,
		const bool bInvalidateOnCompletion,
		FBPOnRequestFailedWithLog OnFailedDelegate);

	/**
	 * Constructs a request to start an online session
	 * @param WorldContextObject	The world context object, IF YOU SEE THIS IN BLUEPRINTS, YOU ARE DOING SOMETHING WRONG >:(
	 * @param bInvalidateOnCompletion	Whether to invalidate the request when it's completed
	 * @param OnSucceededDelegate	Delegate to call when the request succeeds
	 * @param OnFailedDelegate		Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedSessions|Sessions", meta =
		(WorldContext = "WorldContextObject", Keywords = "Make, Create, New", DisplayName = "Construct Online Start Session Request",
			AdvancedDisplay = "bInvalidateOnCompletion", bInvalidateOnCompletion = "true"))
	static UPARAM(DisplayName = "Request") UEnhancedOnlineRequest_StartSession* ConstructOnlineStartSessionRequest(
		UObject* WorldContextObject,
		const bool bInvalidateOnCompletion,
		FBPOnStartSessionRequestSucceeded OnSucceededDelegate,
		FBPOnRequestFailedWithLog OnFailedDelegate);
};
