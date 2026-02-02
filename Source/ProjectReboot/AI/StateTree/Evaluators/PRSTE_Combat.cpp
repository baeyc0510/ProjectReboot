// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTE_Combat.h"
#include "StateTreeExecutionContext.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

void FPRStateTreeEvaluator_Combat::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Context에서 Controller와 Pawn 캐싱
	InstanceData.CachedController = Cast<APRAIController>(Context.GetOwner());

	if (IsValid(InstanceData.CachedController.Get()))
	{
		if (UPRAIConfig* AIConfig = InstanceData.CachedController->GetAIConfig())
		{
			InstanceData.CombatConfig = AIConfig->CombatConfig;
		}

		InstanceData.CachedPawn = Cast<APREnemyCharacter>(InstanceData.CachedController->GetPawn());
	}
}

void FPRStateTreeEvaluator_Combat::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (IsValid(InstanceData.CachedController.Get()))
	{
		InstanceData.CachedController->SetCombatTarget(nullptr);
	}

	InstanceData.CachedController.Reset();
	InstanceData.CachedPawn.Reset();
}

void FPRStateTreeEvaluator_Combat::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Controller에서 타겟 동기화 (Perception 이벤트로 관리됨)
	if (IsValid(InstanceData.CachedController.Get()))
	{
		InstanceData.TargetActor = InstanceData.CachedController->GetCombatTarget();
		InstanceData.bHasValidTarget = IsValid(InstanceData.TargetActor);
	}
	else
	{
		InstanceData.TargetActor = nullptr;
		InstanceData.bHasValidTarget = false;
	}

	if (InstanceData.bHasValidTarget)
	{
		InstanceData.DistanceToTarget = CalculateDistanceToTarget(InstanceData);
	}
	else
	{
		InstanceData.DistanceToTarget = MAX_FLT;
	}
}

float FPRStateTreeEvaluator_Combat::CalculateDistanceToTarget(const FInstanceDataType& Data) const
{
	if (!IsValid(Data.CachedPawn.Get()) || !IsValid(Data.CachedController.Get()))
	{
		return MAX_FLT;
	}

	if (!IsValid(Data.TargetActor))
	{
		return MAX_FLT;
	}

	return FVector::Dist(Data.CachedPawn->GetActorLocation(), Data.TargetActor->GetActorLocation());
}
