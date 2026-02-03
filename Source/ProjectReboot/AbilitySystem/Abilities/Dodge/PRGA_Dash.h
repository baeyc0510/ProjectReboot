// PRGA_Dash.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Dash.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_ApplyRootMotionConstantForce;

/**
 * 대시 어빌리티
 * - 입력 기반 발동
 * - 루트 모션 대시 몽타주 재생
 * - Event.Dash 이벤트 발송 (저스트 회피 트리거용)
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Dash : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Dash();

	/*~ UGameplayAbility Interface ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_Dash Interface ~*/

	// 대시 이벤트 발송 (Event.Dash)
	void SendDashEvent();

	// 대시 몽타주 재생
	void PlayDashMontage();

	// MovementInput 기반 대시 방향 계산 (캐릭터 로컬 기준)
	FVector GetDashDirection() const;

	// 대시 방향 계산 (월드 공간)
	FVector GetDashWorldDirection() const;

	// 대시 방향에 맞는 몽타주 태그 반환
	FGameplayTag GetDashMontageTag(const FVector& LocalDirection) const;

	// 대시 몽타주 선택
	UAnimMontage* GetDashMontage() const;

	// 대시 이동 적용
	void ApplyDashMovement();

	// 몽타주 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnDashMovementFinished();

protected:
	// 대시 몽타주 오버라이드
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashMontageOverride;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float MontagePlayRate = 1.0f;

	// 대시 이동 강도
	UPROPERTY(EditDefaultsOnly, Category = "Dash|Movement")
	float DashStrength = 1500.0f;

	// 대시 이동 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Dash|Movement")
	float DashDuration = 0.3f;

	// 대시 중 중력 적용 여부
	UPROPERTY(EditDefaultsOnly, Category = "Dash|Movement")
	bool bEnableGravityDuringDash = false;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> DashMoveTask;
};
