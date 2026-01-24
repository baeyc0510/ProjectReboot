// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "PRSTT_MoveToIdealRange.generated.h"

class AAIController;
class UAITask_MoveTo;

/**
 * 타겟과 이상적 거리 유지하며 이동
 */

// Instance Data: 상태 저장용
USTRUCT()
struct FPRStateTreeTask_MoveToIdealRangeInstanceData
{
	GENERATED_BODY()

	// 타겟 액터 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 이상적 거리 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float IdealRange = 500.f;

	// 도착 허용 오차
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 50.f;

	// Strafe 가능 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bCanStrafe = false;

	// 캐시된 Controller (상태)
	UPROPERTY()
	TWeakObjectPtr<AAIController> CachedController;

	// MoveTo Task (상태)
	UPROPERTY()
	TObjectPtr<UAITask_MoveTo> MoveToTask;

	// 태스크 완료 여부 (상태)
	bool bTaskCompleted = false;

	// 태스크 성공 여부 (상태)
	bool bTaskSucceeded = false;
};

// Task 구조체
USTRUCT(meta = (DisplayName = "PR Move To Ideal Range"))
struct PROJECTREBOOT_API FPRStateTreeTask_MoveToIdealRange : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeTask_MoveToIdealRangeInstanceData;

	/*~ FStateTreeTaskBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// 이상적 위치 계산
	FVector CalculateIdealLocation(const FInstanceDataType& Data, APawn* Pawn, UWorld* World) const;
};
