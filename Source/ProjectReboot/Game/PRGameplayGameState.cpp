// Fill out your copyright notice in the Description page of Project Settings.

#include "PRGameplayGameState.h"

#include "StateTreeEvents.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/StateTree/PRRoomStateTreeComponent.h"

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

	OnEventCountChanged.Broadcast(EventTag, Count);
}

int32 APRGameplayGameState::GetEventCount(const FGameplayTag& EventTag) const
{
	if (const int32* Count = CurrentEventCounts.Find(EventTag))
	{
		return *Count;
	}

	return 0;
}

void APRGameplayGameState::ResetEventCount(const FGameplayTag& EventTag)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	CurrentEventCounts.Remove(EventTag);
	OnEventCountChanged.Broadcast(EventTag, 0);
}
