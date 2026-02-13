// PRGA_Equip.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Equip.generated.h"

class UPREquipActionData;
class UAbilitySystemComponent;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UPREquipmentManagerComponent;

/**
 * 장비 장착 어빌리티
 * - 장비 태그 변경 시 장착 몽타주 재생
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Equip : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Equip();

	/*~ UGameplayAbility Interface ~*/
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 장착 몽타주 재생
	void PlayEquipMontage(UAnimMontage* MontageToPlay);

	// 이벤트 태그에 매칭되는 몽타주 태그 찾기
	bool TryGetMontageTagForEventTag(const FGameplayTag& EventTag, FGameplayTag& OutMontageTag) const;
	
	// 몽타주 종료 처리
	UFUNCTION()
	void HandleMontageFinished();

	// 장비 가시성 숨김 처리(지연 스케줄링)
	void ScheduleHideEquipment(const UPREquipActionData* EquipAction);

	// 장비 가시성 숨김 처리
	void TryHideEquipment(const UPREquipActionData* EquipAction);

	// 다음 프레임에서 장비 숨김 처리
	UFUNCTION()
	void HandleDeferredHideEquipment();

	// 장비 가시성 복구 처리
	void ShowHiddenEquipment();

	// 장비 노출 이벤트 수신
	UFUNCTION()
	void OnShowEquipmentEventReceived(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	FGameplayTag MontageTagOverride; 
	
	// 이벤트 태그 -> 몽타주 태그 매핑
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	TMap<FGameplayTag, FGameplayTag> EventTagToMontageTag;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	float MontagePlayRate = 1.0f;

private:
	// 몽타주 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 장비 노출 이벤트 대기 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ShowEquipmentWaitTask;

	// 숨긴 장비 슬롯 태그
	FGameplayTag HiddenEquipmentSlot;

	// 장비 숨김 여부
	bool bIsEquipmentHidden = false;

	// 장비 매니저 캐시
	TWeakObjectPtr<UPREquipmentManagerComponent> CachedEquipmentManager;

	// 지연 처리용 장착 액션
	TWeakObjectPtr<const UPREquipActionData> PendingEquipAction;

	// 다음 프레임 처리용 타이머
	FTimerHandle HideEquipmentTimerHandle;
};
