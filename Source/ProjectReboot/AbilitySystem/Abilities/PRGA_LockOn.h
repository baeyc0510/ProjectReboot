// PRGA_LockOn.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "ProjectReboot/Combat/Targeting/PRTargetLockComponent.h"
#include "PRGA_LockOn.generated.h"

class UPRTargetLockComponent;
class UAbilitySystemComponent;

/**
 * 락온 어빌리티
 * - 런 중 획득하여 락온 기능 활성화
 * - 어빌리티 부여 시 TargetLockComponent 동적 부착
 * - 어빌리티 제거 시 TargetLockComponent 제거
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_LockOn : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_LockOn();

	/*~ UGameplayAbility Interface ~*/
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	// TargetLockComponent 생성 및 부착
	UPRTargetLockComponent* CreateAndAttachLockComponent(AActor* Owner);

	// TargetLockComponent 제거
	void RemoveLockComponent(AActor* Owner);

	// 락온 상태 태그 이벤트 바인딩
	void BindToASC(UAbilitySystemComponent* InASC);

	// 락온 상태 태그 이벤트 바인딩 해제
	void UnbindFromASC();

	// 락온 상태 태그 변경 처리
	void HandleLockOnStateTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 현재 상태에 따라 락온 활성화 갱신
	void RefreshLockOnEnabled();

protected:
	// 생성할 TargetLockComponent 클래스 (BP 확장 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TSubclassOf<UPRTargetLockComponent> TargetLockComponentClass = UPRTargetLockComponent::StaticClass();

	// 락온 범위 (컴포넌트 기본값 오버라이드)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Settings")
	float LockOnRange = 3000.0f;

	// 락온 시야각 (컴포넌트 기본값 오버라이드)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Settings")
	float LockOnConeAngle = 60.0f;

	// 락온 소요 시간 (컴포넌트 기본값 오버라이드)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Settings")
	float LockOnTime = 0.8f;
	
	// 락온 활성화 기준 상태 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Settings")
	FGameplayTag LockOnStateTag;

private:
	// 생성된 컴포넌트 참조 (제거 시 사용)
	UPROPERTY()
	TWeakObjectPtr<UPRTargetLockComponent> CreatedLockComponent;

	// ASC 참조 (태그 이벤트 해제 시 사용)
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	// 락온 상태 태그 이벤트 핸들
	FDelegateHandle LockOnStateTagHandle;
};
