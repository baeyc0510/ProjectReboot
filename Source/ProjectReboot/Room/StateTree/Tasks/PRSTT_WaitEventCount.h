// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "PRSTT_WaitEventCount.generated.h"

/**
 * 이벤트 카운트 대기 Task Instance Data
 */
USTRUCT()
struct FPRSTT_WaitEventCount_InstanceData
{
	GENERATED_BODY()

	// 대기할 이벤트 태그 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag EventTag;

	// 목표 카운트 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	int32 TargetCount = 0;

	// 목표 달성 시 추가로 발생시킬 이벤트들 (Optional Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	TArray<FGameplayTag> OnCompletionEvents;

	// 태스크 완료 여부 (런타임)
	bool bPendingCompleted = false;
	bool bCompleted = false;

	// 델리게이트 핸들 (런타임)
	FDelegateHandle EventHandle;
};

/**
 * 이벤트 카운트 대기 StateTree Task
 * 특정 이벤트의 카운트가 목표치에 도달할 때까지 대기
 * 목표 달성 시 OnCompletionEvents에 지정된 이벤트들을 자동으로 발생
 */
USTRUCT(meta = (DisplayName = "Wait Event Count", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_WaitEventCount : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_WaitEventCount_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
