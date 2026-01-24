// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTE_Combat.h"
#include "StateTreeExecutionContext.h"
#include "Perception/AIPerceptionComponent.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"

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
		InstanceData.CachedPerceptionComponent = InstanceData.CachedController->GetPerceptionComponent();
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
	InstanceData.CachedPerceptionComponent.Reset();
}

void FPRStateTreeEvaluator_Combat::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Perception에서 타겟 업데이트
	UpdateTargetFromPerception(InstanceData);

	if (IsValid(InstanceData.CachedController.Get()))
	{
		InstanceData.bHasValidTarget = IsValid(InstanceData.CachedController->GetCombatTarget());	
	}
	else
	{
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

void FPRStateTreeEvaluator_Combat::UpdateTargetFromPerception(FInstanceDataType& Data) const
{
	if (!IsValid(Data.CachedController.Get()) || !IsValid(Data.CachedPerceptionComponent.Get()))
	{
		return;
	}

	// Perception에서 플레이어 타겟 검색
	TArray<AActor*> PerceivedActors;
	Data.CachedPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

	for (AActor* Actor : PerceivedActors)
	{
		if (APRPlayerCharacter* Player = Cast<APRPlayerCharacter>(Actor))
		{
			if (!IsValid(Data.TargetActor))
			{
				Data.CachedController->SetCombatTarget(Player);
				Data.TargetActor = Player;
				Data.bHasValidTarget = true;
				break;
			}
		}
	}

	// 타겟이 인식 범위를 벗어났는지 확인
	if (IsValid(Data.TargetActor) && !PerceivedActors.Contains(Data.TargetActor))
	{
		Data.CachedController->SetCombatTarget(nullptr);
		Data.TargetActor = nullptr;
		Data.bHasValidTarget = false;
	}
}
