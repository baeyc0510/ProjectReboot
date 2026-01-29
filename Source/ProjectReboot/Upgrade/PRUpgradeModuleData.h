// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueliteActionData.h"
#include "PRUpgradeTypes.h"
#include "PRUpgradeModuleData.generated.h"

class UTexture2D;

/**
 * 업그레이드 모듈 데이터
 * RogueliteActionData를 감싸 비용 시스템만 추가
 */
UCLASS()
class PROJECTREBOOT_API UPRUpgradeModuleData : public UDataAsset
{
	GENERATED_BODY()

public:
	/*~ Action ~*/

	// 실제 적용할 액션 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<URogueliteActionData> ActionData;

	/*~ Cost System ~*/

	// 사용할 화폐 종류 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FGameplayTag CurrencyTag;

	// 비용 스케일링 모드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	EUpgradeCostScalingMode CostScalingMode = EUpgradeCostScalingMode::Linear;

	// 기본 비용 (레벨 1)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0"))
	float BaseCost = 100.0f;

	// 레벨당 증가량 (Linear) 또는 배율 (Exponential)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0"))
	float CostGrowth = 50.0f;

	// Custom 모드용 커브
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost", meta = (EditCondition = "CostScalingMode == EUpgradeCostScalingMode::Custom"))
	FRuntimeFloatCurve CostCurve;

public:
	// 액션 데이터 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	URogueliteActionData* GetActionData() const { return ActionData; }

	// 표시 이름 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	FText GetDisplayName() const;

	// 설명 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	FText GetDescription() const;

	// 아이콘 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	UTexture2D* GetIcon() const;

	// 최대 스택 수 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetMaxStacks() const;

	// 특정 레벨의 비용 계산
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	float GetCostForLevel(int32 Level) const;

	// 현재 레벨에서 다음 레벨로의 비용 (CurrentLevel + 1의 비용)
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	float GetNextLevelCost(int32 CurrentLevel) const;
};
