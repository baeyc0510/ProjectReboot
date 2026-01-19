// PRWeaponAttributeSet.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PRWeaponAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 무기 관련 어트리뷰트 세트
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRWeaponAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UPRWeaponAttributeSet();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

public:
    // 현재 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    FGameplayAttributeData Ammo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, Ammo)

    // 최대 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    FGameplayAttributeData MaxAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxAmmo)

    // 예비 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    FGameplayAttributeData ReserveAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, ReserveAmmo)

    // 최대 예비 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Ammo")
    FGameplayAttributeData MaxReserveAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxReserveAmmo)

    // 발사 속도 (발/분)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData FireRate;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, FireRate)

    // 재장전 시간 (초)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData ReloadTime;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, ReloadTime)

    // 기본 데미지
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    FGameplayAttributeData BaseDamage;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, BaseDamage)

    // 데미지 배율
    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    FGameplayAttributeData DamageMultiplier;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, DamageMultiplier)

    /*~ Beam Weapon Attributes ~*/

    // 현재 에너지
    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    FGameplayAttributeData Energy;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, Energy)

    // 최대 에너지
    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    FGameplayAttributeData MaxEnergy;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxEnergy)

    // 초당 에너지 소모량
    UPROPERTY(BlueprintReadOnly, Category = "Energy")
    FGameplayAttributeData EnergyDrainRate;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, EnergyDrainRate)

    /*~ Missile Weapon Attributes ~*/

    // 장전된 미사일 수
    UPROPERTY(BlueprintReadOnly, Category = "Missile")
    FGameplayAttributeData LoadedMissiles;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, LoadedMissiles)

    // 최대 미사일 장전 수
    UPROPERTY(BlueprintReadOnly, Category = "Missile")
    FGameplayAttributeData MaxLoadedMissiles;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxLoadedMissiles)

    // 폭발 반경
    UPROPERTY(BlueprintReadOnly, Category = "Missile")
    FGameplayAttributeData ExplosionRadius;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, ExplosionRadius)
};