// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "PRSTT_ActivateAbilityByTag.generated.h"

class UAbilitySystemComponent;
struct FAbilityEndedData;

/**
 * AbilityTag로 어빌리티를 실행하는 Task
 */

// Instance Data: 상태 저장용
USTRUCT()
struct FPRStateTreeTask_ActivateAbilityByTagInstanceData
{
	GENERATED_BODY()

	// 실행할 어빌리티 태그
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTagContainer AbilityTags;

	// 어빌리티 종료까지 대기할지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bWaitAbilityEnd = false;

	// ASC 캐시 (상태)
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// 발동 대상 AbilitySpecHandle (상태)
	FGameplayAbilitySpecHandle SpecHandleToActivate;

	// 어빌리티 종료 델리게이트 핸들 (상태)
	FDelegateHandle AbilityEndedHandle;

	// 태스크 완료 여부 (상태)
	bool bTaskCompleted = false;

	// 태스크 성공 여부 (상태)
	bool bTaskSucceeded = false;
};

// Task 구조체
USTRUCT(meta = (DisplayName = "PR Activate Ability By Tag"))
struct PROJECTREBOOT_API FPRStateTreeTask_ActivateAbilityByTag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRStateTreeTask_ActivateAbilityByTagInstanceData;

	/*~ FStateTreeTaskBase Interface ~*/
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	// 델리게이트 정리
	void ClearAbilityEndDelegate(FInstanceDataType& Data) const;
};
