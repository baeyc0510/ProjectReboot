// PRDamageExecCalc.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PRDamageExecCalc.generated.h"

/**
 * 데미지 이펙트 계산용 ExecCalc
 */
UCLASS(Blueprintable)
class PROJECTREBOOT_API UPRDamageExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRDamageExecCalc();

	/*~ UGameplayEffectExecutionCalculation Interface ~*/

	// 데미지 계산 실행
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	// 동적 태그에서 무기 타입 태그 추출
	bool GetWeaponTypeTag(const FGameplayTagContainer& InTags, FGameplayTag& OutWeaponTypeTag) const;

	// 동적 태그에서 데미지 타입 태그 추출
	bool GetDamageTypeTag(const FGameplayTagContainer& InTags, FGameplayTag& OutDamageTypeTag) const;

	// 무기 타입에 따른 데미지 배율 계산
	float GetWeaponTypeMultiplier(const FGameplayTag& WeaponTypeTag) const;

	// 데미지 타입에 따른 데미지 배율 계산
	float GetDamageTypeMultiplier(const FGameplayTag& DamageTypeTag) const;

	// 무기 타입에 따른 실드 데미지 배율 계산
	float GetShieldDamageMultiplier(const FGameplayTag& WeaponTypeTag) const;
	
protected:
	// 무기 타입별 배율 (에디터에서 추가/수정)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Configuration")
	TMap<FGameplayTag, float> WeaponTypeMultipliers;

	// 데미지 타입별 공격력 배율 (에디터에서 추가/수정)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Configuration")
	TMap<FGameplayTag, float> DamageTypeMultipliers;

	// 데미지 타입별 실드 추가 데미지 배율 (에디터에서 추가/수정)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Configuration")
	TMap<FGameplayTag, float> ShieldDamageMultipliers;
};
