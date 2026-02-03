// PRGameplayAbility_WeaponFire.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/ProjectReboot.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGameplayAbility_WeaponFire.generated.h"

class UWeaponInstance;
class UPREquipmentManagerComponent;

/**
 * 무기 발사 어빌리티 기본 클래스
 * WeaponInstance 참조 및 공통 유틸리티 제공
 */
UCLASS(Abstract)
class PROJECTREBOOT_API UPRGameplayAbility_WeaponFire : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGameplayAbility_WeaponFire();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
protected:
	// WeaponInstance 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UWeaponInstance* GetWeaponInstance() const;

	// 타입별 WeaponInstance 조회 헬퍼
	template<typename T>
	T* GetWeaponInstanceAs() const
	{
		return Cast<T>(GetWeaponInstance());
	}

	// EquipmentManager 조회
	UPREquipmentManagerComponent* GetEquipmentManager() const;

	// Hitscan 유틸리티: 카메라 기준 라인 트레이스
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	TArray<FHitResult> PerformHitscan(float Range = 10000.0f) const;

	// Hitscan with spread
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	TArray<FHitResult> PerformHitscanWithSpread(float SpreadAngle, float Range = 10000.0f) const;

	// 데미지 적용 (GameplayEffect 사용)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyWeaponDamage(const FHitResult& HitResult, int32 PenetrationCount = 0);

protected:
	// 카메라 시작점 및 방향 계산
	void GetCameraAimInfo(FVector& OutStart, FVector& OutDirection) const;

	// 조준점 계산 (카메라 기준 먼 거리)
	FVector GetAimPoint(float Distance = 10000.0f) const;
	
public:
	// 무기 슬롯 태그 (어떤 슬롯의 무기를 사용할지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponSlotTag;
	
	// 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 트레이스 채널
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<ECollisionChannel> TraceChannel = PRCollision::AttackTrace;
};
