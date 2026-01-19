// PRWeaponAttributeSet.cpp

#include "PRWeaponAttributeSet.h"

UPRWeaponAttributeSet::UPRWeaponAttributeSet()
{
	// Bullet
	InitAmmo(30.0f);
	InitMaxAmmo(30.0f);
	InitReserveAmmo(90.0f);
	InitMaxReserveAmmo(90.0f);
	InitFireRate(600.0f);
	InitReloadTime(2.0f);
	InitBaseDamage(25.0f);
	InitDamageMultiplier(1.0f);

	// Beam
	InitEnergy(100.0f);
	InitMaxEnergy(100.0f);
	InitEnergyDrainRate(20.0f);

	// Missile
	InitLoadedMissiles(4.0f);
	InitMaxLoadedMissiles(4.0f);
	InitExplosionRadius(300.0f);
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
	// Energy 클램핑
	else if (Attribute == GetEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEnergy());
	}
	// LoadedMissiles 클램핑
	else if (Attribute == GetLoadedMissilesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxLoadedMissiles());
	}
	// 음수 방지 (Max 값들, 비율들)
	else if (Attribute == GetMaxEnergyAttribute() ||
			 Attribute == GetEnergyDrainRateAttribute() ||
			 Attribute == GetMaxLoadedMissilesAttribute() ||
			 Attribute == GetExplosionRadiusAttribute())
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
	// MaxEnergy 변경 시 Energy 재클램핑
	else if (Attribute == GetMaxEnergyAttribute())
	{
		if (GetEnergy() > NewValue)
		{
			SetEnergy(NewValue);
		}
	}
	// MaxLoadedMissiles 변경 시 LoadedMissiles 재클램핑
	else if (Attribute == GetMaxLoadedMissilesAttribute())
	{
		if (GetLoadedMissiles() > NewValue)
		{
			SetLoadedMissiles(NewValue);
		}
	}
}