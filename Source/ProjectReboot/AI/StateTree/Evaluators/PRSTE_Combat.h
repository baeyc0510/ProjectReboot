// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionTypes.h"
#include "ProjectReboot/AI/PRAIConfig.h"
#include "ProjectReboot/AI/StateTree/PRStateTreeEvaluatorBase.h"
#include "PRSTE_Combat.generated.h"

struct FPRAICombatConfig;
class APRAIController;
class APREnemyCharacter;
class UAIPerceptionComponent;

/**
 * 전투 데이터 Evaluator
 * Perception 이벤트 바인딩 및 전투 상태 계산
 */
UCLASS(DisplayName = "Combat Evaluator")
class PROJECTREBOOT_API UPRSTE_Combat : public UPRStateTreeEvaluatorBase
{
	GENERATED_BODY()

public:
	/*~ Outputs ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	FPRAICombatConfig CombatConfig;
	
	// 타겟까지 거리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	float DistanceToTarget = 0.f;

	// 유효한 타겟 존재 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	bool bHasValidTarget = false;

	// 현재 타겟 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<AActor> TargetActor;

protected:
	/*~ UPRStateTreeEvaluatorBase Interface ~*/
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	/*~ Perception ~*/
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	// 거리 계산
	float CalculateDistanceToTarget() const;
	
private:
	// 캐시된 참조
	UPROPERTY()
	TWeakObjectPtr<APRAIController> CachedController;

	UPROPERTY()
	TWeakObjectPtr<APREnemyCharacter> CachedPawn;

	UPROPERTY()
	TWeakObjectPtr<UAIPerceptionComponent> CachedPerceptionComponent;
};