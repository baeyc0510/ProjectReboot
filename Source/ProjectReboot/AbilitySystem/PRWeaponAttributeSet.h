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
    /*~ Common Weapon Attributes ~*/
    // 발사 속도 (발/분)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData FireRate;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, FireRate)
    
    // 재장전 시간 (초)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData ReloadTime;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, ReloadTime)
    
    // 기본 데미지
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData BaseDamage;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, BaseDamage)

    // 데미지 배율
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData DamageMultiplier;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, DamageMultiplier)
    
    // 현재 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData Ammo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, Ammo)

    // 최대 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData MaxAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxAmmo)

    // 예비 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData ReserveAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, ReserveAmmo)

    // 최대 예비 탄약
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData MaxReserveAmmo;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxReserveAmmo)
    
    // 최대 관통 횟수 (0이면 관통 없음)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData MaxPenetration;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, MaxPenetration)

    // 관통 시 데미지 감쇄율 (예: 0.8이면 관통할 때마다 데미지가 20%씩 감소)
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData PenetrationDamageFalloff;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, PenetrationDamageFalloff)
    
    // 폭발 반경, 플라즈마 크기
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    FGameplayAttributeData EffectRadius;
    ATTRIBUTE_ACCESSORS(UPRWeaponAttributeSet, EffectRadius)
};