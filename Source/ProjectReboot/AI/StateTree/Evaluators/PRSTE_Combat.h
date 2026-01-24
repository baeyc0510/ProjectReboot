// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "ProjectReboot/AI/PRAIConfig.h"
#include "PRSTE_Combat.generated.h"

class APRAIController;
class APREnemyCharacter;
class UAIPerceptionComponent;

/**
 * 전투 데이터 Evaluator
 * Perception 상태 폴링 및 전투 상태 계산
 */

// Instance Data: 상태 저장용
USTRUCT()
struct FPRStateTreeEvaluator_CombatInstanceData
{
	GENERATED_BODY()

	/*~ Outputs ~*/

	// 전투 설정
	UPROPERTY(EditAnywhere, Category = "Output")
	FPRAICombatConfig CombatConfig;

	// 타겟까지 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.f;

	// 유효한 타겟 존재 여부
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasValidTarget = false;

	// 타겟 액터
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor;

	/*~ Cached References (상태) ~*/

	UPROPERTY()
	TWeakObjectPtr<APRAIController> CachedController;

	UPROPERTY()
	TWeakObjectPtr<APREnemyCharacter> CachedPawn;

	UPROPERTY()
	TWeakObjectPtr<UAIPerceptionComponent> CachedPerceptionComponent;
};

// Evaluator 구조체
USTRUCT(meta = (DisplayName = "PR Combat Evaluator"))
struct PROJECTREBOOT_API FPRStateTreeEvaluator_Combat : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeEvaluator_CombatInstanceData;

	/*~ FStateTreeEvaluatorBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	// 거리 계산
	float CalculateDistanceToTarget(const FInstanceDataType& Data) const;

	// Perception 기반 타겟 업데이트
	void UpdateTargetFromPerception(FInstanceDataType& Data) const;
};
