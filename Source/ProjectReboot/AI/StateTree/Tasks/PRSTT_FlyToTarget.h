// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AI/StateTree/PRStateTreeTaskBase.h"
#include "PRSTT_FlyToTarget.generated.h"

/**
 * 비행 AI용 직접 이동 태스크
 * NavMesh 없이 타겟 위치로 직접 비행
 */
UCLASS()
class PROJECTREBOOT_API UPRSTT_FlyToTarget : public UPRStateTreeTaskBase
{
	GENERATED_BODY()

public:
	/*~ UPRStateTreeTaskBase Interfaces ~*/
	virtual void OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

protected:
	// 목표 위치 계산
	FVector CalculateDestination(APawn* Pawn) const;

public:
	/*~ Parameters ~*/
    
	// 이동 목표 액터
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;
    
	// 타겟과의 이상적 거리
	UPROPERTY(EditAnywhere, Category = "Input")
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

private:
	// 캐시된 목표 위치
	FVector CachedDestination;
    
	// 캐시된 Pawn
	TWeakObjectPtr<APawn> CachedPawn;
};