// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTC_CanActivateAbilityByTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

bool FPRStateTreeCondition_CanActivateAbilityByTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.AbilityTags.IsValid())
	{
		return InstanceData.bInvert;
	}

	// Context Owner에서 ASC 조회
	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller))
	{
		return InstanceData.bInvert;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		return InstanceData.bInvert;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!IsValid(ASC))
	{
		return InstanceData.bInvert;
	}

	// Tag에 매칭되는 AbilitySpec 검색
	TArray<FGameplayAbilitySpec*> MatchingAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InstanceData.AbilityTags, MatchingAbilities);

	if (MatchingAbilities.IsEmpty())
	{
		return InstanceData.bInvert;
	}

	// 첫 번째 매칭 Spec의 발동 가능 여부 체크 (쿨다운, 코스트, BlockedTag 등)
	const FGameplayAbilitySpec* Spec = MatchingAbilities[0];
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	const bool bCanActivate = Spec->Ability && Spec->Ability->CanActivateAbility(Spec->Handle, ActorInfo);
	
	return InstanceData.bInvert ? !bCanActivate : bCanActivate;
}
