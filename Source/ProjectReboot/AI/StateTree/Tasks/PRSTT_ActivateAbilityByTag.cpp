// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_ActivateAbilityByTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FPRStateTreeTask_ActivateAbilityByTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AbilityTags.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

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

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!IsValid(ASC))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CachedASC = ASC;
	InstanceData.SpecHandleToActivate = FGameplayAbilitySpecHandle(); // 핸들 리셋
	
	// Tag 기반 Spec검색
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InstanceData.AbilityTags, MatchingGameplayAbilities);
	
	if (MatchingGameplayAbilities.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_ActivateAbility: %s에 해당하는 활성 가능 어빌리티가 없음"), *InstanceData.AbilityTags.ToString());
		return EStateTreeRunStatus::Failed;
	}
	
	// 첫 번째 스펙 사용
	if (FGameplayAbilitySpec* AbilitySpec = MatchingGameplayAbilities[0])
	{
		InstanceData.SpecHandleToActivate = AbilitySpec->Handle;
	}
	
	// 어빌리티 종료 이벤트 바인딩
	if (InstanceData.bWaitAbilityEnd)
	{
		InstanceData.AbilityEndedHandle = ASC->OnAbilityEnded.AddLambda(
			[&InstanceData](const FAbilityEndedData& AbilityEndedData)
			{
				if (InstanceData.CachedASC.IsValid() && AbilityEndedData.AbilitySpecHandle == InstanceData.SpecHandleToActivate)
				{
					InstanceData.bTaskCompleted = true;
					InstanceData.bTaskSucceeded = !AbilityEndedData.bWasCancelled;
				}
			});
	}
	
	const bool bActivated = ASC->TryActivateAbility(InstanceData.SpecHandleToActivate);
	
	if (!bActivated)
	{
		ClearAbilityEndDelegate(InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.bWaitAbilityEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_ActivateAbilityByTag::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bTaskCompleted)
	{
		ClearAbilityEndDelegate(InstanceData);
		return InstanceData.bTaskSucceeded ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_ActivateAbilityByTag::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ClearAbilityEndDelegate(InstanceData);
	InstanceData.SpecHandleToActivate = FGameplayAbilitySpecHandle(); // 핸들 리셋
}

void FPRStateTreeTask_ActivateAbilityByTag::ClearAbilityEndDelegate(FInstanceDataType& Data) const
{
	if (Data.CachedASC.IsValid() && Data.AbilityEndedHandle.IsValid())
	{
		Data.CachedASC->OnAbilityEnded.Remove(Data.AbilityEndedHandle);
	}

	Data.AbilityEndedHandle.Reset();
	Data.CachedASC.Reset();
}
