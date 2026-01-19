#include "PRGameplayTags.h"

/*~ Input Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Input, "Input");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Crouch, "Input.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Aim, "Input.Aim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Fire, "Input.Fire");

/*~ Equipment Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment, "Equipment");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Bullet, "Equipment.Weapon.Type.Bullet");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Beam, "Equipment.Weapon.Type.Beam");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Missile, "Equipment.Weapon.Type.Missile");

/*~ Character States ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_State, "State");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Sprint, "State.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Crouch, "State.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Aiming, "State.Aiming");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_CannotFire, "State.Weapon.CannotFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair, "State.Weapon.Crosshair");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair_Rifle, "State.Weapon.Crosshair.Rifle");

/*~ SetByCaller Tags - Weapon Attributes ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller, "SetByCaller");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon, "SetByCaller.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_Ammo, "SetByCaller.Weapon.Ammo");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_MaxAmmo, "SetByCaller.Weapon.MaxAmmo");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_ReserveAmmo, "SetByCaller.Weapon.ReserveAmmo");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_MaxReserveAmmo, "SetByCaller.Weapon.MaxReserveAmmo");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_FireRate, "SetByCaller.Weapon.FireRate");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_ReloadTime, "SetByCaller.Weapon.ReloadTime");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_BaseDamage, "SetByCaller.Weapon.BaseDamage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_DamageMultiplier, "SetByCaller.Weapon.DamageMultiplier");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_Energy, "SetByCaller.Weapon.Energy");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_MaxEnergy, "SetByCaller.Weapon.MaxEnergy");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_EnergyDrainRate, "SetByCaller.Weapon.EnergyDrainRate");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_LoadedMissiles, "SetByCaller.Weapon.LoadedMissiles");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_MaxLoadedMissiles, "SetByCaller.Weapon.MaxLoadedMissiles");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_ExplosionRadius, "SetByCaller.Weapon.ExplosionRadius");