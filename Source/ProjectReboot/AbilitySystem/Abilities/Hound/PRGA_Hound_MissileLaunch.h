// PRGA_Hound_MissileLaunch.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Hound_MissileLaunch.generated.h"

class APRHoundAOEZone;
class AActor;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;

/**
 * 하운드 미사일 발사 어빌리티
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Hound_MissileLaunch : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Hound_MissileLaunch();

	/*~ UGameplayAbility Interface ~*/

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 공격 이벤트 수신 → AOE Zone 스폰
	UFUNCTION()
	void OnAttackEventReceived(FGameplayEventData Payload);

	// AOE Zone 스폰
	void SpawnAOEZone();

	// 타겟 위치 계산
	FVector CalculateTargetLocation() const;

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	/*~ Config - AOE Zone ~*/

	// 스폰할 AOE Zone 클래스
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile")
	TSubclassOf<APRHoundAOEZone> AOEZoneClass;

	// 데미지 GE 클래스 (Duration + Period 타입)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지 (틱당)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Damage")
	float BaseDamage = 10.f;

	// 화염 영역 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Damage")
	float DamageDuration = 5.f;

	// 틱 데미지 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Damage")
	float DamagePeriod = 0.5f;

	// 데미지 대상 클래스 필터 (비어있으면 전체)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Target")
	TSubclassOf<AActor> TargetActorClass;

	/*~ Config - Montage ~*/

	// 몽타주 직접 설정 여부 (false: 태그 기반 조회)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Montage")
	bool bUseMontageOverride = false;

	// 시전 몽타주 (직접 설정)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Montage", meta = (EditCondition = "bUseMontageOverride"))
	TObjectPtr<UAnimMontage> CastMontage;

	// 시전 몽타주 태그 (태그 기반 조회)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Montage", meta = (EditCondition = "!bUseMontageOverride"))
	FGameplayTag CastMontageTag;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Montage")
	float MontagePlayRate = 1.0f;

	/*~ Config - Telegraph ~*/

	// Telegraph 경고 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Telegraph")
	float TelegraphDuration = 1.5f;

	// AOE 반지름
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Telegraph")
	float StrikeRadius = 300.f;

	// 경고 색상
	UPROPERTY(EditDefaultsOnly, Category = "HoundMissile|Telegraph")
	FLinearColor TelegraphColor = FLinearColor::Red;

private:
	// 몽타주 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 이벤트 대기 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEventTask;

	// 재생할 몽타주 (런타임 결정)
	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;
};
