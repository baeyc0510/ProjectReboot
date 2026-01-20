// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTE_Combat.h"

#include "StateTreeExecutionContext.h"
#include "Perception/AIPerceptionComponent.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

void UPRSTE_Combat::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);

	// Context에서 Controller와 Pawn 캐싱
	CachedController = Cast<APRAIController>(Context.GetOwner());
	
	if (IsValid(CachedController.Get()))
	{
		if (UPRAIConfig* AIConfig = CachedController->GetAIConfig())
		{
			CombatConfig = AIConfig->CombatConfig;
		}
		
		CachedPawn = Cast<APREnemyCharacter>(CachedController->GetPawn());
		CachedPerceptionComponent = CachedController->GetPerceptionComponent();

		// Perception 이벤트 바인딩
		if (IsValid(CachedPerceptionComponent.Get()))
		{
			CachedPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UPRSTE_Combat::OnPerceptionUpdated);
		}
	}
}

void UPRSTE_Combat::TreeStop(FStateTreeExecutionContext& Context)
{
	// Perception 이벤트 해제
	if (IsValid(CachedPerceptionComponent.Get()))
	{
		CachedPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &UPRSTE_Combat::OnPerceptionUpdated);
	}

	CachedController.Reset();
	CachedPawn.Reset();
	CachedPerceptionComponent.Reset();
	TargetActor = nullptr;

	Super::TreeStop(Context);
}

void UPRSTE_Combat::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	bHasValidTarget = IsValid(TargetActor);

	if (bHasValidTarget)
	{
		DistanceToTarget = CalculateDistanceToTarget();
	}
	else
	{
		DistanceToTarget = MAX_FLT;
	}
}

void UPRSTE_Combat::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		// 타겟 획득
		if (!IsValid(TargetActor))
		{
			TargetActor = Actor;
		}
	}
	else
	{
		// 타겟 상실
		if (TargetActor == Actor)
		{
			TargetActor = nullptr;
		}
	}
}

float UPRSTE_Combat::CalculateDistanceToTarget() const
{
	if (!IsValid(CachedPawn.Get()) || !IsValid(TargetActor))
	{
		return MAX_FLT;
	}

	return FVector::Dist(CachedPawn->GetActorLocation(), TargetActor->GetActorLocation());
}