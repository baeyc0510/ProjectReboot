// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_SetRoomDoorsInteractable.h"

#include "ProjectReboot/Room/PRRoomController.h"

EStateTreeRunStatus FPRStateTreeTask_SetRoomDoorsInteractable::EnterState(
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
	
	Data.RoomController->SetDoorsInteractable(Data.bInteractable);

	return EStateTreeRunStatus::Succeeded;
}
