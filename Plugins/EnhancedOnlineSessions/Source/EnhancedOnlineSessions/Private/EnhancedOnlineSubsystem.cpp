// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineSubsystem.h"

void UEnhancedOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BindOnlineDelegates();

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UEnhancedOnlineSubsystem::BindOnlineDelegates()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	BindSessionDelegates(Sessions);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	BindIdentityDelegates(Identity);
}

void UEnhancedOnlineSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEnhancedOnlineSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create the subsystem if there are no child classes
	return ChildClasses.Num() == 0;
}