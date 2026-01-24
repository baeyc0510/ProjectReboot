// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_FaceTarget.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/KismetMathLibrary.h"

EStateTreeRunStatus FPRStateTreeTask_FaceTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CachedPawn = Pawn;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_FaceTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	APawn* Pawn = InstanceData.CachedPawn.Get();
	if (!IsValid(Pawn) || !IsValid(InstanceData.TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 타겟 방향 계산
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector TargetLocation = InstanceData.TargetActor->GetActorLocation();
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);
    
	const FRotator CurrentRotation = Pawn->GetActorRotation();
	const FRotator TargetRotation = FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll); // Yaw만 회전
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InstanceData.RotationRate / 360.f * 10.f);
    
	Pawn->SetActorRotation(NewRotation);
	
	// 태스크가 매 Tick 계속 실행되어야 하므로 Running 반환
	return EStateTreeRunStatus::Running;
}
