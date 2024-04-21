// Copyright © 2024 MajorT. All rights reserved.


#include "EnhancedOnlineSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

class IOnlineSubsystem;

void UEnhancedOnlineSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEnhancedOnlineSessionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEnhancedOnlineSessionsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	return ChildClasses.Num() == 0;
}
