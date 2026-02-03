// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTE_Combat.h"
#include "StateTreeExecutionContext.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"

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

		if (IsValid(InstanceData.CachedPawn.Get()))
		{
			InstanceData.CachedASC = Cast<UPRAbilitySystemComponent>(InstanceData.CachedPawn->GetAbilitySystemComponent());
		}
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
	InstanceData.CachedASC.Reset();
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
		CalculateDistanceToTarget(InstanceData);
	}
	else
	{
		InstanceData.DistanceToTarget = MAX_FLT;
		InstanceData.DistanceToTarget2D = MAX_FLT;
	}

	// 체력 비율 업데이트
	if (IsValid(InstanceData.CachedASC.Get()))
	{
		const float Health = InstanceData.CachedASC->GetNumericAttribute(UPRCommonAttributeSet::GetHealthAttribute());
		const float MaxHealth = InstanceData.CachedASC->GetNumericAttribute(UPRCommonAttributeSet::GetMaxHealthAttribute());
		InstanceData.HealthPercent = (MaxHealth > 0.f) ? (Health / MaxHealth) : 0.f;
	}
	else
	{
		InstanceData.HealthPercent = 1.f;
	}

	// 사망 상태 업데이트
	InstanceData.bIsDead = IsValid(InstanceData.CachedPawn.Get()) && InstanceData.CachedPawn->IsDead();
}

void FPRStateTreeEvaluator_Combat::CalculateDistanceToTarget(FInstanceDataType& OutData) const
{
	if (!IsValid(OutData.CachedPawn.Get()) || !IsValid(OutData.CachedController.Get()))
	{
		OutData.DistanceToTarget =  MAX_FLT;
		OutData.DistanceToTarget2D =  MAX_FLT;
	}

	if (!IsValid(OutData.TargetActor))
	{
		OutData.DistanceToTarget =  MAX_FLT;
		OutData.DistanceToTarget2D =  MAX_FLT;
	}
	
	const FVector OwnerLocation = OutData.CachedPawn->GetActorLocation();
	const FVector TargetLocation = OutData.TargetActor->GetActorLocation();
	
	OutData.DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
	OutData.DistanceToTarget2D = FVector::Dist2D(OwnerLocation,TargetLocation);
}