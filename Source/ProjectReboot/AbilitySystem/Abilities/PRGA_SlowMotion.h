// PRGA_SlowMotion.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_SlowMotion.generated.h"

/**
 * 슬로우 모션 어빌리티
 * - Event.JustDodge 이벤트로 트리거
 * - 글로벌 타임 딜레이션으로 월드 슬로우
 * - 플레이어는 CustomTimeDilation으로 정상 속도 유지
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
	/*~ UPRGA_SlowMotion Interface ~*/

	// 슬로우 모션 적용
	void ApplySlowMotion();

	// 슬로우 모션 해제
	void RestoreNormalTime();

	// 슬로우 모션 종료 콜백
	UFUNCTION()
	void OnSlowMotionDurationExpired();

protected:
	// 글로벌 타임 딜레이션 (0.0 ~ 1.0, 낮을수록 느림)
	UPROPERTY(EditDefaultsOnly, Category = "SlowMotion")
	float GlobalTimeDilation = 0.3f;

	// 플레이어 타임 딜레이션 보정 (1.0 = 정상 속도 유지)
	UPROPERTY(EditDefaultsOnly, Category = "SlowMotion")
	float PlayerTimeDilationMultiplier = 1.0f;

	// 슬로우 모션 지속 시간 (실제 시간 기준)
	UPROPERTY(EditDefaultsOnly, Category = "SlowMotion")
	float SlowMotionDuration = 2.0f;

private:
	// 원래 글로벌 타임 딜레이션 저장
	float OriginalGlobalTimeDilation = 1.0f;

	// 원래 플레이어 타임 딜레이션 저장
	float OriginalPlayerTimeDilation = 1.0f;
};
