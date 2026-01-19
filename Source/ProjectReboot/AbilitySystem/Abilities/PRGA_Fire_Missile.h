// PRGA_Fire_Missile.h
#pragma once

#include "CoreMinimal.h"
#include "PRGameplayAbility_WeaponFire.h"
#include "PRGA_Fire_Missile.generated.h"

class UMissileWeaponInstance;
class APRProjectile;

/**
 * 미사일 무기 발사 어빌리티 (Projectile 스폰)
 * 단발 발사, Projectile Actor 스폰
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
	// 미사일 발사 수행
	void FireMissile();

	// Projectile 스폰
	AActor* SpawnProjectile(const FTransform& SpawnTransform);

	// MissileWeaponInstance 조회
	UMissileWeaponInstance* GetMissileWeapon() const;

protected:
	// 스폰할 Projectile 클래스 (Instance에 없을 경우 폴백)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	TSubclassOf<AActor> DefaultProjectileClass;

	// 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	float ProjectileSpeed = 3000.0f;
};
