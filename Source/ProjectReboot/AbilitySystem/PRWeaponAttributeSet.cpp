// PRWeaponAttributeSet.cpp

#include "PRWeaponAttributeSet.h"

UPRWeaponAttributeSet::UPRWeaponAttributeSet()
{
	InitAmmo(30.0f);
	InitMaxAmmo(30.0f);
	InitReserveAmmo(90.0f);
	InitMaxReserveAmmo(90.0f);
	InitFireRate(600.0f);
	InitReloadTime(2.0f);
	InitBaseDamage(25.0f);
	InitDamageMultiplier(1.0f);
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
			 Attribute == GetBaseDamageAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	// 배율 최소값
	else if (Attribute == GetDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
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