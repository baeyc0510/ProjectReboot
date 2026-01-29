// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PRExecCalc_SetByCallerMappings.generated.h"

USTRUCT(BlueprintType)
struct FPRSetByCallerAttributeMapEntry
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayAttribute Attribute;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EGameplayModOp::Type> ModOp = EGameplayModOp::AddBase;
};
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRExecCalc_SetByCallerMappings : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	/*~ UGameplayEffectExecutionCalculation Interface ~*/

	// 어트리뷰트 맵핑 적용 실행
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;


protected:
	// SetByCaller 태그가 존재할 때만 어트리뷰트 적용
	void TryApplySetByCaller(
		const FGameplayEffectSpec& Spec,
		const FGameplayTag& SetByCallerTag,
		const FGameplayAttribute& TargetAttribute,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput,
		EGameplayModOp::Type ModOp) const;
	
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<FGameplayTag,FPRSetByCallerAttributeMapEntry> SetByCallerAttributeMap;
};
