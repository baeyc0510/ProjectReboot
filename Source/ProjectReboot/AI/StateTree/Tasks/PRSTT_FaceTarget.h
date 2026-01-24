// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "PRSTT_FaceTarget.generated.h"

/**
 * 타겟 방향으로 회전하는 Task
 * 지속적으로 타겟을 바라봄
 */

// Instance Data: 상태 저장용
USTRUCT()
struct FPRStateTreeTask_FaceTargetInstanceData
{
	GENERATED_BODY()

	// 타겟 액터 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 회전 속도
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float RotationRate = 120.f;

	// 캐시된 Pawn (상태)
	UPROPERTY()
	TWeakObjectPtr<APawn> CachedPawn;
};

// Task 구조체: 상태 없음 (const 함수들)
USTRUCT(meta = (DisplayName = "PR Face Target"))
struct PROJECTREBOOT_API FPRStateTreeTask_FaceTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeTask_FaceTargetInstanceData;

	/*~ FStateTreeTaskBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
