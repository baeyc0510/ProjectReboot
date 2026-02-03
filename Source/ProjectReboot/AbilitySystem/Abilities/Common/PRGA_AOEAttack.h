// PRGA_AOEAttack.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "ProjectReboot/Combat/PRTraceTypes.h"
#include "PRGA_AOEAttack.generated.h"

class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 범위 공격 어빌리티 (AOE)
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_AOEAttack : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_AOEAttack();

	/*~ UGameplayAbility Interfaces ~*/

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_AOEAttack Interfaces ~*/

	// AOE 공격 몽타주 재생
	void PlayAttackMontage();

	// 공격 이벤트 수신
	UFUNCTION()
	void OnAttackEventReceived(FGameplayEventData Payload);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	// 몽타주 블렌드아웃 콜백
	UFUNCTION()
	void OnMontageBlendOut();

	// 몽타주 취소 콜백
	UFUNCTION()
	void OnMontageCancelled();

	// AOE 트레이스 수행
	void PerformAOETrace(const FGameplayEventData* TriggerEventData);

	// AOE 중심 위치 계산
	FVector ResolveTargetLocation(const FGameplayEventData* TriggerEventData) const;

	// 메시 컴포넌트 조회
	USkeletalMeshComponent* GetMeshComponent() const;

	// 데미지 적용
	void ApplyAOEDamage(const FHitResult& HitResult);

protected:
	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Montage")
	float MontagePlayRate = 1.0f;

	// 공격 이벤트 다중 수신 허용 여부
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	bool bAllowMultipleEvents = false;

	// 이벤트 타겟 위치 사용 여부
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	bool bUseEventTargetLocation = true;

	// 소켓 위치 사용 여부
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	bool bUseTraceCenterSocket = false;

	// 트레이스 중심 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	FName TraceCenterSocketName;

	// 트레이스 중심 소켓 오프셋 (소켓 로컬)
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	FVector TraceCenterSocketOffset = FVector::ZeroVector;

	// 타겟 위치 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	FVector TargetLocationOffset = FVector::ZeroVector;

	// 트레이스 설정
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Trace")
	FPRTraceSettings TraceSettings;

	// 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Damage")
	float BaseDamage = 10.0f;

private:
	// 몽타주 태스크
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	// 이벤트 대기 태스크
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitEventTask;

	// 중복 히트 방지용 캐시
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
