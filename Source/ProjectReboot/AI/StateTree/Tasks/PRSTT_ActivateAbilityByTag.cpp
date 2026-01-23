// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_ActivateAbilityByTag.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

void UPRSTT_ActivateAbilityByTag::OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (!AbilityTags.IsValid())
	{
		FinishTask(false);
		return;
	}

	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller))
	{
		FinishTask(false);
		return;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		FinishTask(false);
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!IsValid(ASC))
	{
		FinishTask(false);
		return;
	}

	CachedASC = ASC;
	SpecHandleToActivate = FGameplayAbilitySpecHandle(); // 핸들 리셋
	
	// Tag 기반 Spec검색
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags,MatchingGameplayAbilities);
	
	if (MatchingGameplayAbilities.IsEmpty())
	{
		UE_LOG(LogTemp,Warning,TEXT("PRSTT_ActivateAbility: %s에 해당하는 활성 가능 어빌리티가 없음"),*AbilityTags.ToString());
		FinishTask(false);
		return;
	}
	
	// 첫 번째 스펙 사용
	if (FGameplayAbilitySpec* AbilitySpec = MatchingGameplayAbilities[0])
	{
		SpecHandleToActivate = AbilitySpec->Handle;
	}
	
	// 어빌리티 종료 이벤트 바인딩
	if (bWaitAbilityEnd)
	{
		AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UPRSTT_ActivateAbilityByTag::HandleAbilityEnded);
	}
	
	const bool bActivated = ASC->TryActivateAbility(SpecHandleToActivate);
	
	if (!bActivated)
	{
		ClearAbilityEndDelegate();
		FinishTask(false);
		return;
	}

	if (!bWaitAbilityEnd)
	{
		FinishTask(true);
	}
}

void UPRSTT_ActivateAbilityByTag::OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	ClearAbilityEndDelegate();
	SpecHandleToActivate = FGameplayAbilitySpecHandle(); // 핸들 리셋
}

void UPRSTT_ActivateAbilityByTag::HandleAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (!bWaitAbilityEnd)
	{
		return;
	}

	if (!CachedASC.IsValid() || AbilityEndedData.AbilitySpecHandle != SpecHandleToActivate)
	{
		return;
	}

	ClearAbilityEndDelegate();
	FinishTask(!AbilityEndedData.bWasCancelled);
}

void UPRSTT_ActivateAbilityByTag::ClearAbilityEndDelegate()
{
	if (CachedASC.IsValid() && AbilityEndedHandle.IsValid())
	{
		CachedASC->OnAbilityEnded.Remove(AbilityEndedHandle);
	}

	AbilityEndedHandle.Reset();
	CachedASC.Reset();
}
