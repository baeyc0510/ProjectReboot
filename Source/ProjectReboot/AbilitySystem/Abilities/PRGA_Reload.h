// PRGA_Reload.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/Abilities/PRGameplayAbility_WeaponFire.h"
#include "PRGA_Reload.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UWeaponInstance;

/**
 * 리로드 어빌리티
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Reload : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Reload();

	/*~ UGameplayAbility Interface ~*/
	// 리로드 가능 여부 확인
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 리로드 시작
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 리로드 종료 처리
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_Reload Interfaces ~*/
	// 리로드 몽타주 재생
	void PlayReloadMontage(UAnimMontage* MontageToPlay);

	// 리로드 몽타주 선택
	UAnimMontage* GetReloadMontage() const;

	// 무기 타입별 리로드 태그 반환
	FGameplayTag GetReloadMontageTagByWeaponType() const;

	// 리로드 타임 가져오기
	float GetReloadTime() const;

	// 지연 기반 리로드 시작
	void StartReloadDelay(float Delay);

	// 리로드 완료
	void FinishReload();

	// 리로드 취소
	void CancelReload();

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();

	// 지연 종료 콜백
	UFUNCTION()
	void OnReloadDelayFinished();

	// WeaponInstance 조회
	UWeaponInstance* GetWeaponInstance() const;


protected:
	// 무기 슬롯 태그 (어떤 슬롯의 무기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponSlotTag;
	
	// 리로드 몽타주 오버라이드
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	TObjectPtr<UAnimMontage> ReloadMontageOverride;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	float MontagePlayRate = 1.0f;

	// ReloadTime 기반 재생 속도 보정 여부
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	bool bMatchReloadTime = true;

	// ReloadTime 최소값 (bMatchReloadTime 사용 시)
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	float MinReloadTimeWhenMatch = 0.1f;

private:
	// 몽타주 태스크
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	// 지연 태스크
	UPROPERTY()
	UAbilityTask_WaitDelay* ReloadDelayTask;
};
