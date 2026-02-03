// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Leviathan_OrbitalRain.generated.h"

class APROrbitalStrike;
class UAbilityTask_PlayMontageAndWait;
class UGameplayEffect;

/**
 * 레비아탄 오비탈 레인 어빌리티
 * 맵을 선회하며 지면에 시간차 광역기 다수 생성
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Leviathan_OrbitalRain : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_Leviathan_OrbitalRain();

	/*~ UGameplayAbility Interface ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Strike 순차 스폰 시작
	void SpawnStrikes();

	// 랜덤 지면 위치 계산
	FVector CalculateRandomGroundLocation() const;

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

	// 스폰할 OrbitalStrike 클래스
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain")
	TSubclassOf<APROrbitalStrike> OrbitalStrikeClass;

	// 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Damage")
	float BaseDamage = 30.f;

	// 시전 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Montage")
	TObjectPtr<UAnimMontage> CastMontage;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Montage")
	float MontagePlayRate = 1.0f;

	// 최소 Strike 수
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	int32 MinStrikeCount = 5;

	// 최대 Strike 수
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	int32 MaxStrikeCount = 8;

	// Strike 스폰 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	float SpawnInterval = 0.3f;

	// Telegraph 경고 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	float TelegraphDuration = 1.5f;

	// Strike 반지름
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	float StrikeRadius = 300.f;

	// 경고 색상
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Strike")
	FLinearColor TelegraphColor = FLinearColor::Red;

	// 스폰 영역 반경 (캐릭터 중심 기준)
	UPROPERTY(EditDefaultsOnly, Category = "OrbitalRain|Spawn")
	float SpawnAreaRadius = 1500.f;

private:
	// 몽타주 태스크
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 스폰 타이머 핸들 배열 (정리용)
	TArray<FTimerHandle> SpawnTimerHandles;
};
