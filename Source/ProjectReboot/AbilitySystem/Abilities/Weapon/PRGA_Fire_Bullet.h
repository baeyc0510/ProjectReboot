// PRGA_Fire_Bullet.h
#pragma once

#include "CoreMinimal.h"
#include "PRGameplayAbility_WeaponFire.h"
#include "PRGA_Fire_Bullet.generated.h"

class UBulletWeaponInstance;

/**
 * 탄환 무기 발사 어빌리티 (Hitscan)
 * 연사 지원, 탄퍼짐 적용
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Fire_Bullet : public UPRGameplayAbility_WeaponFire
{
	GENERATED_BODY()

public:
	UPRGA_Fire_Bullet();

	/*~ UGameplayAbility Interface ~*/
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	/*~ UPRGameplayAbility_WeaponFire Interface ~*/
	virtual void OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 단발 발사 수행
	void FireOnce();

	// BulletWeaponInstance 조회
	UBulletWeaponInstance* GetBulletWeapon() const;

protected:
	// 탄퍼짐 각도 (AttributeSet에서 가져올 수도 있음)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet")
	float BaseSpreadAngle = 1.0f;
};
