// PRWeaponAttributeSet.cpp

#include "PRWeaponAttributeSet.h"

UPRWeaponAttributeSet::UPRWeaponAttributeSet()
{
	// 공통
	InitFireRate(600.0f);
	InitReloadTime(2.0f);
	InitBaseDamage(1.0f);
	InitDamageMultiplier(1.0f);

	// 탄약
	InitAmmo(0.0f);
	InitMaxAmmo(0.0f);
	InitReserveAmmo(0.0f);
	InitMaxReserveAmmo(0.0f);

	// 관통
	InitMaxPenetration(0.0f);
	InitPenetrationDamageFalloff(1.0f);

	// 효과 반경
	InitEffectRadius(100.0f);
}

void UPRWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Ammo 클램핑
	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
	// ReserveAmmo 클램핑
	else if (Attribute == GetReserveAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxReserveAmmo());
	}
	// 음수 방지
	else if (Attribute == GetMaxAmmoAttribute() ||
			 Attribute == GetMaxReserveAmmoAttribute() ||
			 Attribute == GetFireRateAttribute() ||
			 Attribute == GetReloadTimeAttribute() ||
			 Attribute == GetBaseDamageAttribute() ||
			 Attribute == GetMaxPenetrationAttribute() ||
			 Attribute == GetEffectRadiusAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	// 배율 최소값
	else if (Attribute == GetDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	// 관통 데미지 감쇄율 클램핑
	else if (Attribute == GetPenetrationDamageFalloffAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.01f, 1.0f);
	}
}

void UPRWeaponAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// MaxAmmo 변경 시 현재 Ammo 재클램핑
	if (Attribute == GetMaxAmmoAttribute())
	{
		if (GetAmmo() > NewValue)
		{
			SetAmmo(NewValue);
		}
	}
	// MaxReserveAmmo 변경 시 ReserveAmmo 재클램핑
	else if (Attribute == GetMaxReserveAmmoAttribute())
	{
		if (GetReserveAmmo() > NewValue)
		{
			SetReserveAmmo(NewValue);
		}
	}
}