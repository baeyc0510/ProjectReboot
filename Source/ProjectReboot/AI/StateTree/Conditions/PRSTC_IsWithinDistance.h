// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "PRSTC_IsWithinDistance.generated.h"

/**
 * 두 액터 사이 거리가 특정 값 이내인지 확인
 */

// Instance Data
USTRUCT()
struct FPRStateTreeCondition_IsWithinDistanceInstanceData
{
	GENERATED_BODY()

	// 소스 액터
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> SourceActor;

	// 타겟 액터
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	// 최대 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxDistance = 500.f;

	// 조건 반전 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

// Condition 구조체
USTRUCT(meta = (DisplayName = "Is Actor Within Distance"))
struct PROJECTREBOOT_API FPRStateTreeCondition_IsWithinDistance : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeCondition_IsWithinDistanceInstanceData;

	/*~ FStateTreeConditionBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
