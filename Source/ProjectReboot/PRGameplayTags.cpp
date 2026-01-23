#include "PRGameplayTags.h"

/*~ Input Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Input, "Input");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Crouch, "Input.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Aim, "Input.Aim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Fire, "Input.Fire");

/*~ Equipment Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment, "Equipment");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type, "Equipment.Weapon.Type");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Bullet, "Equipment.Weapon.Type.Bullet");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Beam, "Equipment.Weapon.Type.Beam");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Missile, "Equipment.Weapon.Type.Missile");

/*~ Damage Type Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType, "DamageType");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Kinetic, "DamageType.Kinetic");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Plasma, "DamageType.Plasma");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Explosion, "DamageType.Explosion");

/*~ Character States ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_State, "State");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Sprint, "State.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Crouch, "State.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Aiming, "State.Aiming");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_CannotFire, "State.Weapon.CannotFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair, "State.Weapon.Crosshair");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair_Rifle, "State.Weapon.Crosshair.Rifle");

/*~ Event Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Event, "Event");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Death, "Event.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Hit, "Event.Hit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Attack, "Event.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Room, "Event.Room");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Room_Clear, "Event.Room.Clear");


/*~ Montage Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage, "Montage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Attack, "Montage.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Death, "Montage.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Death_Front, "Montage.Death.Front");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Death_Right,"Montage.Death.Right");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Death_Back,"Montage.Death.Back");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Death_Left,"Montage.Death.Left");

UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Hit, "Montage.Hit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Hit_Front, "Montage.Hit.Front");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Hit_Right, "Montage.Hit.Right");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Hit_Back, "Montage.Hit.Back");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Hit_Left, "Montage.Hit.Left");

/*~ Ability Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability, "Ability");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack, "Ability.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Die, "Ability.Die");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Active, "Ability.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Active_Aim, "Ability.Active.Aim");

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
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_MaxPenetration, "SetByCaller.Weapon.MaxPenetration");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_PenetrationDamageFalloff, "SetByCaller.Weapon.PenetrationDamageFalloff");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Weapon_EffectRadius, "SetByCaller.Weapon.EffectRadius");

/*~ SetByCaller Tags - Combat Attributes ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Combat_Damage, "SetByCaller.Combat.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Combat_PenetrationCount, "SetByCaller.Combat.PenetrationCount");