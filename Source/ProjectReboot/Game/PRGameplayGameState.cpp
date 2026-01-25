// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameState.h"

#include "StateTreeEvents.h"
#include "Components/StateTreeComponent.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRRoomStateTreeComponent.h"

void APRGameplayGameState::NotifyRoomEnter(APRRoomController* NewRoom)
{
	CurrentRoomController = NewRoom;
	CurrentEventCounts.Empty();
}

void APRGameplayGameState::SendRoomEvent(const FGameplayTag& EventTag)
{
	if (CurrentRoomController && CurrentRoomController->GetStateTreeComponent())
	{
		FStateTreeEvent STEvent;
		STEvent.Tag = EventTag;
		CurrentRoomController->GetStateTreeComponent()->SendStateTreeEvent(STEvent);
	}
}

void APRGameplayGameState::AddEventCount(const FGameplayTag& EventTag, int32 Delta)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	int32& Count = CurrentEventCounts.FindOrAdd(EventTag);
	Count = FMath::Max(0, Count + Delta);
}

int32 APRGameplayGameState::GetEventCount(const FGameplayTag& EventTag) const
{
	if (const int32* Count = CurrentEventCounts.Find(EventTag))
	{
		return *Count;
	}

	return 0;
}