// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Leviathan_PlasmaWave.generated.h"

class APRPlasmaWave;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;

/**
 * 레비아탄 플라즈마 웨이브 어빌리티
 * 백덤블링 모션 후 액터 아래 지면에서 부채꼴 웨이브 3개 발사
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Leviathan_PlasmaWave : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Leviathan_PlasmaWave();

	/*~ UGameplayAbility Interface ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 웨이브 스폰 (부채꼴 3방향)
	void SpawnWaves();

	// 단일 웨이브 스폰
	void SpawnSingleWave(const FVector& SpawnLocation, const FVector& Direction, const FGameplayEffectSpecHandle& InDamageSpec);

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

protected:
	/*~ Config ~*/

	// 스폰할 PlasmaWave 클래스
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave")
	TSubclassOf<APRPlasmaWave> PlasmaWaveClass;

	// 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave|Damage")
	float BaseDamage = 20.f;

	// 백덤블링 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave|Montage")
	TObjectPtr<UAnimMontage> BackflipMontage;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave|Montage")
	float MontagePlayRate = 1.0f;

	// 부채꼴 각도 (좌/우 오프셋)
	UPROPERTY(EditDefaultsOnly, Category = "PlasmaWave|Spawn")
	float SpreadAngle = 30.f;

private:
	// 몽타주 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 이벤트 대기 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEventTask;
};
