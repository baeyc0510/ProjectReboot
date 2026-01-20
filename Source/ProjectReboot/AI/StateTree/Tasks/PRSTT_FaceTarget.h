// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AI/StateTree/PRStateTreeTaskBase.h"
#include "PRSTT_FaceTarget.generated.h"

/**
 * 타겟 방향으로 회전하는 Task
 * 지속적으로 타겟을 바라봄
 */
UCLASS(DisplayName = "FaceTarget")
class PROJECTREBOOT_API UPRSTT_FaceTarget : public UPRStateTreeTaskBase
{
	GENERATED_BODY()

public:
	// 타겟 액터 (바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 회전 속도
	UPROPERTY(EditAnywhere, Category = "Input")
	float RotationRate = 120.f;

protected:
	virtual void NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};