// PRWeaponExecCalc.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PRWeaponExecCalc.generated.h"

/**
 * 무기 어트리뷰트 SetByCaller 적용용 ExecCalc
 */
UCLASS()
class PROJECTREBOOT_API UPRWeaponExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRWeaponExecCalc();

	/*~ UGameplayEffectExecutionCalculation Interface ~*/

	// 무기 어트리뷰트 적용 실행
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	// SetByCaller 태그가 존재할 때만 어트리뷰트 적용
	void TryApplySetByCaller(
		const FGameplayEffectSpec& Spec,
		const FGameplayTag& SetByCallerTag,
		const FGameplayAttribute& TargetAttribute,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const;
};
