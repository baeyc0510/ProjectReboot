// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "PRSTT_Patrol.generated.h"

class AAIController;
class UAITask_MoveTo;
class APREnemyCharacter;

/**
 * 스폰 위치 주변을 배회하는 Patrol Task
 */

// Instance Data
USTRUCT()
struct FPRStateTreeTask_PatrolInstanceData
{
	GENERATED_BODY()

	// Enemy Character (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<APREnemyCharacter> EnemyCharacter;

	// 배회 반경
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float PatrolRadius = 500.f;

	// 도착 허용 오차
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 50.f;

	// 각 지점에서 대기 시간
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float WaitTimeAtPoint = 2.f;

	// 캐시된 Controller (상태)
	UPROPERTY()
	TWeakObjectPtr<AAIController> CachedController;

	// MoveTo Task (상태)
	UPROPERTY()
	TObjectPtr<UAITask_MoveTo> MoveToTask;

	// 대기 중인지
	bool bIsWaiting = false;

	// 대기 경과 시간
	float WaitElapsedTime = 0.f;
};

// Task 구조체
USTRUCT(meta = (DisplayName = "PR Patrol"))
struct PROJECTREBOOT_API FPRStateTreeTask_Patrol : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeTask_PatrolInstanceData;

	/*~ FStateTreeTaskBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// 랜덤 배회 위치 생성
	FVector GenerateRandomPatrolLocation(const FInstanceDataType& Data, UWorld* World) const;

	// 다음 위치로 이동 시작
	void StartMoveToNextLocation(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};
