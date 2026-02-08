// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "PRSTC_CheckEventCount.generated.h"

/**
 * 이벤트 카운트 비교 모드
 */
UENUM(BlueprintType)
enum class EPREventCountComparison : uint8
{
	// 크거나 같음 (>=)
	GreaterOrEqual,
	// 같음 (==)
	Equal,
	// 작거나 같음 (<=)
	LessOrEqual,
	// 큼 (>)
	Greater,
	// 작음 (<)
	Less
};

/**
 * 이벤트 카운트 체크 Condition Instance Data
 */
USTRUCT()
struct FPRSTC_CheckEventCount_InstanceData
{
	GENERATED_BODY()

	// 체크할 이벤트 태그 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag EventTag;

	// 비교할 목표 카운트 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	int32 TargetCount = 0;

	// 비교 모드 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	EPREventCountComparison ComparisonMode = EPREventCountComparison::GreaterOrEqual;
};

/**
 * 이벤트 카운트 체크 StateTree Condition
 * GameState의 이벤트 카운트를 목표 값과 비교
 */
USTRUCT(meta = (DisplayName = "Check Event Count", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeCondition_CheckEventCount : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTC_CheckEventCount_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
