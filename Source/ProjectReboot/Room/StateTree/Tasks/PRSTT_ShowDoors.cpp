// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_ShowDoors.h"

#include "ProjectReboot/Room/PRRoomController.h"

EStateTreeRunStatus FPRStateTreeTask_ShowDoors::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!IsValid(Data.RoomController))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 문 표시 및 활성화
	Data.RoomController->ShowDoors();

	return EStateTreeRunStatus::Succeeded;
}
