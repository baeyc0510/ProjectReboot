// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTC_CheckEventCount.h"

#include "StateTreeExecutionContext.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"

bool FPRStateTreeCondition_CheckEventCount::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	// EventTag 유효성 검사
	if (!Data.EventTag.IsValid())
	{
		return false;
	}

	// GameState 가져오기
	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>();
	if (!IsValid(GameState))
	{
		return false;
	}

	// 현재 카운트 조회
	const int32 CurrentCount = GameState->GetEventCount(Data.EventTag);

	// 비교 모드에 따라 판정
	switch (Data.ComparisonMode)
	{
	case EPREventCountComparison::GreaterOrEqual:
		return CurrentCount >= Data.TargetCount;

	case EPREventCountComparison::Equal:
		return CurrentCount == Data.TargetCount;

	case EPREventCountComparison::LessOrEqual:
		return CurrentCount <= Data.TargetCount;

	case EPREventCountComparison::Greater:
		return CurrentCount > Data.TargetCount;

	case EPREventCountComparison::Less:
		return CurrentCount < Data.TargetCount;

	default:
		return false;
	}
}
