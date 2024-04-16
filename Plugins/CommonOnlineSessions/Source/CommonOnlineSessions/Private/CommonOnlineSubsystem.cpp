// Copyright © 2024 MajorT. All rights reserved.


#include "CommonOnlineSubsystem.h"

#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AssetRegistry/AssetData.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Engine/AssetManager.h"

/************************************************************************************
 * Common Online Session Requests													*
 ************************************************************************************/
#pragma region create_session_request
int32 UCommonOnline_CreateSessionRequest::GetMaxPlayers() const
{
	return MaxPlayerCount;
}

FString UCommonOnline_CreateSessionRequest::GetMapName() const
{
	FAssetData MapAssetData;
	if (UAssetManager::Get().GetPrimaryAssetData(MapID, MapAssetData))
	{
		return MapAssetData.PackageName.ToString();
	}
	else
	{
		return FString();
	}
}

FString UCommonOnline_CreateSessionRequest::ConstructTravelURL(bool bWithServerTravel) const
{
	FString CombinedExtraArgs;

	if (OnlineMode == ECommonSessionOnlineMode::LAN)
	{
		CombinedExtraArgs += TEXT("?bIsLanMatch");
	}

	if (OnlineMode != ECommonSessionOnlineMode::Offline && bWithServerTravel)
	{
		CombinedExtraArgs += TEXT("?listen");
	}

	return FString::Printf(TEXT("%s%s"),
		*GetMapName(),
		*CombinedExtraArgs);
}

bool UCommonOnline_CreateSessionRequest::ValidateAndLogErrors(FText& OutError) const
{
#if WITH_SERVER_CODE
	if (GetMapName().IsEmpty())
	{
		OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed"), FText::FromString(MapID.ToString()));
		return false;
	}
	return true;
#else
	OutError = NSLOCTEXT("NetworkErrors", "ClientBuildCannotHost", "Client builds cannot host game sessions.");
	return false;
#endif
}
#pragma endregion

#pragma region find_session_request
int32 UCommonOnlineSearchResult::GetPingInMs() const
{
	return Result.PingInMs;
}

int32 UCommonOnlineSearchResult::GetNumCurrentPlayers() const
{
	return Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
}

int32 UCommonOnlineSearchResult::GetNumMaxPlayers() const
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

FString UCommonOnlineSearchResult::GetSessionNameSafe() const
{
	if (Result.Session.OwningUserId.IsValid())
	{
		if (IOnlineIdentityPtr IdentityPtr = Online::GetIdentityInterface(GetWorld()))
		{
			return IdentityPtr->GetPlayerNickname(*Result.Session.OwningUserId.Get());
		}
	}

	if (!Result.Session.OwningUserName.IsEmpty())
	{
		return Result.Session.OwningUserName;
	}

	if (Result.Session.SessionSettings.Settings.Array().Num() > 0)
	{
		for (auto& Setting : Result.Session.SessionSettings.Settings)
		{
			if (Setting.Key == "FRIENDLYNAME")
			{
				FString OwnerName;
				Setting.Value.Data.GetValue(OwnerName);
				return OwnerName;
			}
		}
	}

	return TEXT("Unknown");
}
#pragma endregion

/************************************************************************************
 * UCommonOnlineSubsystem															*
 ************************************************************************************/

void UCommonOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
}

void UCommonOnlineSubsystem::BindOnlineDelegates()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	BindOnlineIdentityDelegates(OnlineSub);
	BindOnlineSessionDelegates(OnlineSub);
}

void UCommonOnlineSubsystem::Deinitialize()
{
	// Remove all delegate bindings
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		RemoveOnlineIdentityDelegates(OnlineSub);
		RemoveOnlineSessionDelegates(OnlineSub);
	}

	if (GEngine)
	{
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

bool UCommonOnlineSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> SubClasses;
	GetDerivedClasses(GetClass(), SubClasses, false);

	// Only create an instance if this is the only derived class
	return SubClasses.Num() == 0;
}