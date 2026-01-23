// PRGA_Hit.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Hit.generated.h"

enum class EPRHitDirection : uint8;
class UAbilityTask_PlayMontageAndWait;
class UGameplayEffect;

/**
 * 피격 어빌리티
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Hit : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Hit();

	/*~ UGameplayAbility Interfaces ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_Hit Interfaces ~*/

	// 피격 상태 GE 적용
	void ApplyHitState(const FGameplayEventData* TriggerEventData);

	// 피격 몽타주 재생
	void PlayHitMontage(UAnimMontage* MontageToPlay);

	// 피격 몽타주 선택
	UAnimMontage* GetHitMontage(const FGameplayEventData* TriggerEventData) const;

	// 방향별 태그 반환
	FGameplayTag GetHitMontageTagByDirection(EPRHitDirection HitDirection) const;

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	// 피격 상태 GE (Hit 이펙트 적용)
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TSubclassOf<UGameplayEffect> HitStateEffectClass;

	// 피격 몽타주 오버라이드
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> HitMontageOverride;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	float MontagePlayRate = 1.0f;

private:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	// 피격 상태 GE 핸들
	FActiveGameplayEffectHandle HitEffectHandle;
};
