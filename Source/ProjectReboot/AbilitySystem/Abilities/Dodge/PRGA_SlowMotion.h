// PRGA_SlowMotion.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_SlowMotion.generated.h"

/**
 * 슬로우 모션 어빌리티
 * - Event.JustDodge 이벤트로 트리거
 * - 타이밍만 제어하고, 실제 연출은 APRGCN_SlowMotion이 담당
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_SlowMotion : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_SlowMotion();

	/*~ UGameplayAbility Interface ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 슬로우 모션 종료 콜백
	UFUNCTION()
	void OnSlowMotionDurationExpired();

protected:
	// GCN의 GlobalTimeDilation과 동일 값 설정 필요 (WaitDelay 보정에 사용)
	UPROPERTY(EditDefaultsOnly, Category = "SlowMotion")
	float GlobalTimeDilation = 0.3f;

	// 슬로우 모션 지속 시간 (실제 시간 기준)
	UPROPERTY(EditDefaultsOnly, Category = "SlowMotion")
	float SlowMotionDuration = 2.0f;
};
