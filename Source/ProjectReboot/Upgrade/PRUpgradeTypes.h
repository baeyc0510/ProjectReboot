// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRUpgradeTypes.generated.h"

/**
 * 비용 스케일링 모드
 */
UENUM(BlueprintType)
enum class EUpgradeCostScalingMode : uint8
{
	// 고정 비용: 모든 레벨에서 BaseCost
	Fixed,

	// 선형 증가: BaseCost + (level-1) * CostGrowth
	Linear,

	// 지수 증가: BaseCost * CostGrowth^(level-1)
	Exponential,

	// 커스텀 커브 사용
	Custom
};
