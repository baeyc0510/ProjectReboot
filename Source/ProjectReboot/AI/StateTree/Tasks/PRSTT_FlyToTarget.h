// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "PRSTT_FlyToTarget.generated.h"

/**
 * 비행 AI용 직접 이동 태스크 (NavMesh 미사용)
 */

// Instance Data: 상태 저장용
USTRUCT()
struct FPRStateTreeTask_FlyToTargetInstanceData
{
	GENERATED_BODY()

	// 이동 목표 액터
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 타겟과의 이상적 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float IdealRange = 500.f;

	// 타겟 기준 높이 오프셋 (양수 = 위)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float HeightOffset = 300.f;

	// 이동 속도
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float FlySpeed = 600.f;

	// 도착 판정 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 100.f;

	// 캐시된 목표 위치 (상태)
	FVector CachedDestination = FVector::ZeroVector;

	// 캐시된 Pawn (상태)
	UPROPERTY()
	TWeakObjectPtr<APawn> CachedPawn;
};

// Task 구조체
USTRUCT(meta = (DisplayName = "PR Fly To Target"))
struct PROJECTREBOOT_API FPRStateTreeTask_FlyToTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeTask_FlyToTargetInstanceData;

	/*~ FStateTreeTaskBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// 목표 위치 계산
	FVector CalculateDestination(const FInstanceDataType& Data, APawn* Pawn) const;
};
