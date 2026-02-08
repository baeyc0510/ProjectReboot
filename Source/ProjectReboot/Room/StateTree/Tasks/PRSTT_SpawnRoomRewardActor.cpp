// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_SpawnRoomRewardActor.h"

#include "ProjectReboot/Room/PRRoomController.h"

EStateTreeRunStatus FPRStateTreeTask_SpawnRoomRewardActor::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.RewardActorClass)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!IsValid(Data.RoomController))
	{
		return EStateTreeRunStatus::Failed;
	}

	// RoomController를 통해 보상 스폰 (RewardPoolPreset 자동 설정)
	Data.SpawnedActor = Data.RoomController->SpawnReward(Data.RewardActorClass);
	return IsValid(Data.SpawnedActor) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}
