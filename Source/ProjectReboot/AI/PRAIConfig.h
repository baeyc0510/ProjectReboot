// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PRAIConfig.generated.h"

class UStateTree;
/** AI 전투 동작 설정을 위한 데이터 구조체 */
USTRUCT(BlueprintType)
struct FPRAICombatConfig
{
	GENERATED_BODY()

	/*~ Combat Ranges ~*/

	// 공격을 시작할 수 있는 최대 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range")
	float MaxCombatRange = 800.f;

	// AI가 유지하고자 하는 이상적인 거리 (Hold/Strafe 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range")
	float IdealRange = 500.f;

	// 너무 가깝다고 판단되는 최소 거리 (이 거리보다 가까우면 뒤로 물러나거나 Strafe 가중치 증가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range")
	float MinRange = 300.f;

	/*~ Durations ~*/

	// 공격 페이즈 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Duration")
	float AttackPhaseDuration = 3.0f;

	// 휴식/대치 페이즈 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Duration")
	float RestPhaseDuration = 2.0f;
};


/** 적의 모든 정의를 담는 통합 데이터 에셋 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRAIConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 사용할 StateTree
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTree> StateTree;

	// 전투 파라미터 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FPRAICombatConfig CombatConfig;
};