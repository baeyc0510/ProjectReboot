// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "PRStateTreeTaskBase.h"
#include "PRSTT_ActivateAbilityByTag_Deprecated.generated.h"

struct FGameplayAbilitySpecHandle;
class UAbilitySystemComponent;
struct FAbilityEndedData;

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Deprecated,
 * USTRUCT 기반 Task로 이동됨. 성능 비교용으로 사용후 제거 예정
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/**
 * AbilityTag로 어빌리티를 실행하는 Task
 */
UCLASS(DisplayName = "(Deprecated) Activate Ability By Tag")
class PROJECTREBOOT_API UPRSTT_ActivateAbilityByTag : public UPRStateTreeTaskBase
{
	GENERATED_BODY()

public:
	// 실행할 어빌리티 태그
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AbilityTags;

	// 어빌리티 종료까지 대기할지 여부
	UPROPERTY(EditAnywhere)
	bool bWaitAbilityEnd = false;

protected:
	/*~ UPRStateTreeTaskBase Interface ~*/
	virtual void OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private:
	// 어빌리티 종료 콜백
	void HandleAbilityEnded(const FAbilityEndedData& AbilityEndedData);
	// 델리게이트 정리
	void ClearAbilityEndDelegate();

private:
	// ASC 캐시
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// 발동 대상 AbilitySpecHandle
	FGameplayAbilitySpecHandle SpecHandleToActivate;
	
	// 어빌리티 종료 델리게이트 핸들
	FDelegateHandle AbilityEndedHandle;
};
