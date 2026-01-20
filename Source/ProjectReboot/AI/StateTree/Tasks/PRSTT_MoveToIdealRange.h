// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "ProjectReboot/AI/StateTree/PRStateTreeTaskBase.h"
#include "PRSTT_MoveToIdealRange.generated.h"

class UAITask_MoveTo;
class AAIController;
/**
 * 타겟과 이상적 거리 유지하며 이동
 */
UCLASS(DisplayName = "PR Move To Ideal Range")
class PROJECTREBOOT_API UPRSTT_MoveToIdealRange : public UPRStateTreeTaskBase
{
	GENERATED_BODY()

public:
	// 타겟 액터 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 이상적 거리 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	float IdealRange = 500.f;

	// 도착 허용 오차
	UPROPERTY(EditAnywhere, Category = "Input")
	float AcceptanceRadius = 50.f;

protected:
	virtual void OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;\
	virtual void NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	FVector CalculateIdealLocation(APawn* Pawn) const;

	UPROPERTY()
	TWeakObjectPtr<AAIController> CachedController;

	UPROPERTY()
	TObjectPtr<UAITask_MoveTo> MoveToTask;
	
	FAIRequestID CurrentRequestID;
};