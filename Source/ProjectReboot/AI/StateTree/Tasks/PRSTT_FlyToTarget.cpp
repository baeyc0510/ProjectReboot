// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_FlyToTarget.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/PawnMovementComponent.h"

EStateTreeRunStatus FPRStateTreeTask_FlyToTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	InstanceData.CachedPawn = Pawn;
	InstanceData.CachedDestination = CalculateDestination(InstanceData, Pawn);

	// 이미 도착한 경우
	const float DistanceToGoal = FVector::Dist(Pawn->GetActorLocation(), InstanceData.CachedDestination);
	if (DistanceToGoal <= InstanceData.AcceptanceRadius)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_FlyToTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	APawn* Pawn = InstanceData.CachedPawn.Get();
	if (!IsValid(Pawn) || !IsValid(InstanceData.TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 목표 위치 갱신 (타겟이 움직일 수 있으므로)
	InstanceData.CachedDestination = CalculateDestination(InstanceData, Pawn);

	const FVector CurrentLocation = Pawn->GetActorLocation();
	const FVector ToDestination = InstanceData.CachedDestination - CurrentLocation;
	const float DistanceToGoal = ToDestination.Size();

	// 도착 판정
	if (DistanceToGoal <= InstanceData.AcceptanceRadius)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 이동 방향 계산 및 입력 적용
	const FVector MoveDirection = ToDestination.GetSafeNormal();
	Pawn->AddMovementInput(MoveDirection, 1.f);

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_FlyToTarget::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (IsValid(InstanceData.CachedPawn.Get()))
	{
		if (UPawnMovementComponent* MovementComp = InstanceData.CachedPawn->GetMovementComponent())
		{
			MovementComp->StopMovementImmediately();
		}
	}
    
	InstanceData.CachedPawn.Reset();
}

FVector FPRStateTreeTask_FlyToTarget::CalculateDestination(const FInstanceDataType& Data, APawn* Pawn) const
{
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector TargetLocation = Data.TargetActor->GetActorLocation();
    
	// 2D 방향으로 이상적 거리 계산
	const FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal2D();
	FVector Destination = TargetLocation - DirectionToTarget * Data.IdealRange;
    
	// 높이 오프셋 적용
	Destination.Z = TargetLocation.Z + Data.HeightOffset;
    
	return Destination;
}
