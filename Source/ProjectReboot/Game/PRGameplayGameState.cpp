// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameState.h"

#include "StateTreeEvents.h"
#include "Components/StateTreeComponent.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRRoomStateTreeComponent.h"

void APRGameplayGameState::NotifyRoomEnter(APRRoomController* NewRoom)
{
	CurrentRoomController = NewRoom;
	CurrentKillCount = 0; // 킬 카운트 초기화
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