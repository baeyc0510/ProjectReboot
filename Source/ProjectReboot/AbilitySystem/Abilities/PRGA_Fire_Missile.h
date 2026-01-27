// PRGA_Fire_Missile.h
#pragma once

#include "CoreMinimal.h"
#include "PRGameplayAbility_WeaponFire.h"
#include "PRGA_Fire_Missile.generated.h"

class UMissileWeaponInstance;
class APRMissileProjectile;

/**
 * 미사일 무기 발사 어빌리티 (Projectile 스폰)
 * 락온된 모든 타겟에 대해 잔탄이 있는 한 연속 발사
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Fire_Missile : public UPRGameplayAbility_WeaponFire
{
	GENERATED_BODY()

public:
	UPRGA_Fire_Missile();

	/*~ UGameplayAbility Interface ~*/
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	/*~ UPRGameplayAbility_WeaponFire Interface ~*/
	virtual void OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 락온된 모든 타겟에 미사일 발사 (잔탄이 있는 동안)
	void FireAllMissiles();

	// 단일 미사일 발사
	void FireSingleMissile(UMissileWeaponInstance* Weapon, AActor* HomingTarget);

	// Projectile 스폰 및 초기화
	APRMissileProjectile* SpawnProjectile(const FTransform& SpawnTransform);

	// 발사체 초기화 (데미지, 폭발 반경 등 설정)
	void InitializeProjectile(APRMissileProjectile* Projectile, AActor* HomingTarget = nullptr);

	// MissileWeaponInstance 조회
	UMissileWeaponInstance* GetMissileWeapon() const;

protected:
	// 스폰할 Projectile 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	TSubclassOf<APRMissileProjectile> DefaultProjectileClass;

	// 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	float ProjectileSpeed = 3000.0f;

	// 유도 가속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Homing")
	float HomingAccelerationMagnitude = 5000.0f;
};
