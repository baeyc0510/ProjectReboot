// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "PRSTC_CanActivateAbilityByTag.generated.h"

class UAbilitySystemComponent;

/**
 * AbilityTag로 어빌리티 발동 가능 여부를 체크하는 Condition
 * 쿨다운, 코스트, BlockedTag 등 모두 체크됨
 */

// Instance Data
USTRUCT()
struct FPRStateTreeCondition_CanActivateAbilityByTagInstanceData
{
	GENERATED_BODY()

	// 체크할 어빌리티 태그
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTagContainer AbilityTags;

	// 조건 반전 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

// Condition 구조체
USTRUCT(meta = (DisplayName = "PR Can Activate Ability By Tag"))
struct PROJECTREBOOT_API FPRStateTreeCondition_CanActivateAbilityByTag : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeCondition_CanActivateAbilityByTagInstanceData;

	/*~ FStateTreeConditionBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
