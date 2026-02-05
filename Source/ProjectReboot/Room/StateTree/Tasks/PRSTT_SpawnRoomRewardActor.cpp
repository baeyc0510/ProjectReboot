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

	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return EStateTreeRunStatus::Failed;
	}

	FTransform SpawnTransform = FTransform::Identity;
	if (IsValid(Data.SpawnPoint))
	{
		SpawnTransform = Data.SpawnPoint->GetActorTransform();
	}
	else if (Data.bUseRoomControllerTransform && IsValid(Data.RoomController))
	{
		SpawnTransform = Data.RoomController->GetActorTransform();
	}

	Data.SpawnedActor = World->SpawnActor<AActor>(Data.RewardActorClass, SpawnTransform);
	return IsValid(Data.SpawnedActor) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}
