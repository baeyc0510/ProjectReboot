// PRWeaponExecCalc.cpp
#include "PRWeaponExecCalc.h"
#include "GameplayEffectExtension.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRWeaponExecCalc::UPRWeaponExecCalc()
{
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_FireRate, {UPRWeaponAttributeSet::GetFireRateAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_FireRateMultiplier, {UPRWeaponAttributeSet::GetFireRateMultiplierAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_ReloadTime, {UPRWeaponAttributeSet::GetReloadTimeAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_BaseDamage, {UPRWeaponAttributeSet::GetBaseDamageAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_DamageMultiplier, {UPRWeaponAttributeSet::GetDamageMultiplierAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_MaxAmmo, {UPRWeaponAttributeSet::GetMaxAmmoAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_Ammo, {UPRWeaponAttributeSet::GetAmmoAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_MaxReserveAmmo, {UPRWeaponAttributeSet::GetMaxReserveAmmoAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_ReserveAmmo, {UPRWeaponAttributeSet::GetReserveAmmoAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_PenetrationDamageFalloff, {UPRWeaponAttributeSet::GetPenetrationDamageFalloffAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_MaxPenetration, {UPRWeaponAttributeSet::GetMaxPenetrationAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Weapon_EffectRadius, {UPRWeaponAttributeSet::GetEffectRadiusAttribute()});
}