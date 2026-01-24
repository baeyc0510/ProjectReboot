// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTE_Combat_Deprecated.h"

#include "StateTreeExecutionContext.h"
#include "Perception/AIPerceptionComponent.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"

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

	CachedController->SetCombatTarget(nullptr);
	CachedController.Reset();
	CachedPawn.Reset();
	CachedPerceptionComponent.Reset();

	Super::TreeStop(Context);
}

void UPRSTE_Combat::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	if (IsValid(CachedController.Get()))
	{
		bHasValidTarget = IsValid(CachedController->GetCombatTarget());	
	}
	else
	{
		bHasValidTarget = false;
	}

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
	if (!IsValid(CachedController.Get()))
	{
		return;
	}
	
	
	if (Stimulus.WasSuccessfullySensed())
	{
		// 타겟 획득
		if (!IsValid(TargetActor) && Cast<APRPlayerCharacter>(Actor))
		{
			CachedController->SetCombatTarget(Actor);
			TargetActor = Actor;
			bHasValidTarget = true;
		}
	}
	else
	{
		// 타겟 상실
		if (TargetActor == Actor)
		{
			CachedController->SetCombatTarget(nullptr);
			bHasValidTarget = false;
		}
	}
}

float UPRSTE_Combat::CalculateDistanceToTarget() const
{
	if (!IsValid(CachedPawn.Get()) || !IsValid(CachedController.Get()))
	{
		return MAX_FLT;
	}
	
	if (!IsValid(TargetActor))
	{
		return MAX_FLT;
	}

	return FVector::Dist(CachedPawn->GetActorLocation(), TargetActor->GetActorLocation());
}