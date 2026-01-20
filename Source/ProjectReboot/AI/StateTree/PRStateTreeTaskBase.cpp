// Fill out your copyright notice in the Description page of Project Settings.


#include "PRStateTreeTaskBase.h"

EStateTreeRunStatus UPRStateTreeTaskBase::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	SetCachedInstanceDataFromContext(Context);
	RunStatus = EStateTreeRunStatus::Running;
	OnEnterState(Context, Transition);
	return RunStatus;
}

void UPRStateTreeTaskBase::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	OnExitState(Context, Transition);
	
	if (UWorld* CurrentWorld = GetWorld())
	{
		CurrentWorld->GetLatentActionManager().RemoveActionsForObject(this);
		CurrentWorld->GetTimerManager().ClearAllTimersForObject(this);
	}

	ClearCachedInstanceData();
}

void UPRStateTreeTaskBase::StateCompleted(FStateTreeExecutionContext& Context,
	const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates)
{
	OnStateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus UPRStateTreeTaskBase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	NativeReceivedTick(Context, DeltaTime);
	return RunStatus;
}
