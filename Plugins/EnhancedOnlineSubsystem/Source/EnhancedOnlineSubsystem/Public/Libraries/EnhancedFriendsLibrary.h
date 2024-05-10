// Copyright © 2024 MajorT. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedOnlineRequests.h"
#include "EnhancedSessionsLibrary.h"
#include "EnhancedFriendsLibrary.generated.h"

class UEnhancedOnlineRequest_GetFriendsList;

DECLARE_DYNAMIC_DELEGATE_OneParam(FBPOnGetFriendsListCompleted, const TArray<FEnhancedBlueprintFriendInfo>&, FriendsList);
/**
 * Library of functions for interacting with the Enhanced Online Subsystem and friends
 */
UCLASS()
class ENHANCEDONLINESUBSYSTEM_API UEnhancedFriendsLibrary : public UEnhancedSessionsLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Construct a request to get the friends list of the local user
	 * @param WorldContextObject	World context
	 * @param bFilterOnlyOnlineFriends	Should the list only include online friends
	 * @param bFilterOnlyInGameFriends	Should the list only include friends who are in game
	 * @param LocalUserIndex	The index of the local user who made the request
	 * @param bInvalidateOnCompletion	Should the request be invalidated on completion
	 * @param OnCompletedDelegate	Delegate to call when the request completes
	 * @param OnFailedDelegate	Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedFriends|Friends", meta =
		(WorldContext = "WorldContextObject", DisplayName = "Construct Get Friends List Request", Keywords = "Make, Create, New",
			AdvancedDisplay = "bInvalidateOnCompletion, LocalUserIndex"))
	static UEnhancedOnlineRequest_GetFriendsList* ConstructOnlineGetFriendsListRequest(
		UObject* WorldContextObject,
		bool bFilterOnlyOnlineFriends,
		bool bFilterOnlyInGameFriends,
		const int32 LocalUserIndex,
		const bool bInvalidateOnCompletion,
		FBPOnGetFriendsListCompleted OnCompletedDelegate,
		FBPOnRequestFailedWithLog OnFailedDelegate);
	

	/**
	 * Construct a request to find a session of a single friend
	 * @param WorldContextObject	World context
	 * @param FriendId	The friend to search for
	 * @param LocalUserIndex	The index of the local user who made the request
	 * @param bInvalidateOnCompletion	Should the request be invalidated on completion
	 * @param OnCompletedDelegate	Delegate to call when the request completes
	 * @param OnFailedDelegate	Delegate to call when the request fails
	 * @return The request object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Online|EnhancedFriends|Friends", meta =
		(WorldContext = "WorldContextObject", DisplayName = "Construct Find Friend Session Request", Keywords = "Make, Create, New",
			AdvancedDisplay = "bInvalidateOnCompletion, LocalUserIndex"))
	static UEnhancedOnlineRequest_FindFriendSession* ConstructOnlineFindFriendSessionRequest(
		UObject* WorldContextObject,
		const FUniqueNetIdRepl& FriendId,
		const int32 LocalUserIndex,
		const bool bInvalidateOnCompletion,
		FBPOnFindSessionsSuceeeded OnCompletedDelegate,
		FBPOnRequestFailedWithLog OnFailedDelegate);
};
