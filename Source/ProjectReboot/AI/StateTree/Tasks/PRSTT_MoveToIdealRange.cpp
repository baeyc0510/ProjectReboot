// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_MoveToIdealRange.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "Tasks/AITask_MoveTo.h"

EStateTreeRunStatus FPRStateTreeTask_MoveToIdealRange::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller) || !IsValid(InstanceData.TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CachedController = Controller;

	// 이상적 거리 위치 계산
	const FVector Destination = CalculateIdealLocation(InstanceData, Pawn, Context.GetWorld());

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(Destination);
	MoveReq.SetAcceptanceRadius(InstanceData.AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetCanStrafe(InstanceData.bCanStrafe);

	// UAITask_MoveTo 생성 및 설정 (엔진의 PrepareMoveToTask 로직)
	if (!InstanceData.MoveToTask)
	{
		InstanceData.MoveToTask = UAITask::NewAITask<UAITask_MoveTo>(*Controller, *Controller);
	}
	InstanceData.MoveToTask->SetUp(Controller, MoveReq);

	// 태스크 활성화
	if (InstanceData.MoveToTask->IsActive())
	{
		InstanceData.MoveToTask->ConditionalPerformMove();
	}
	else
	{
		InstanceData.MoveToTask->ReadyForActivation();
	}

	// 즉시 종료 확인 (이미 도착한 경우 등)
	if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
	{
		bool bWasMoveSuccessful = InstanceData.MoveToTask->WasMoveSuccessful();
		return bWasMoveSuccessful ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_MoveToIdealRange::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (InstanceData.MoveToTask)
	{
		if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
		{
			bool bWasMoveSuccessful = InstanceData.MoveToTask->WasMoveSuccessful();
			return bWasMoveSuccessful ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_MoveToIdealRange::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (IsValid(InstanceData.CachedController.Get()))
	{
		InstanceData.CachedController->StopMovement();
	}
	InstanceData.CachedController.Reset();
	
	if (IsValid(InstanceData.MoveToTask))
	{
		InstanceData.MoveToTask->EndTask();
	}
	InstanceData.MoveToTask = nullptr;
}

FVector FPRStateTreeTask_MoveToIdealRange::CalculateIdealLocation(const FInstanceDataType& Data, APawn* Pawn, UWorld* World) const
{
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector TargetLocation = Data.TargetActor->GetActorLocation();
	const FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal2D();

	// 타겟에서 이상적 거리만큼 떨어진 위치
	FVector IdealLocation = TargetLocation - DirectionToTarget * Data.IdealRange;

	// NavMesh 위로 보정
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(IdealLocation, NavLocation))
		{
			IdealLocation = NavLocation.Location;
		}
	}

	return IdealLocation;
}
