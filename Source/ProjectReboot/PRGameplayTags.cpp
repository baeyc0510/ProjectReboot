#include "PRGameplayTags.h"

/*~ Input Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Input, "Input");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Crouch, "Input.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Aim, "Input.Aim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Fire, "Input.Fire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Reload, "Input.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Dash, "Input.Dash");

/*~ Equipment Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment, "Equipment");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type, "Equipment.Weapon.Type");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Bullet, "Equipment.Weapon.Type.Bullet");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Beam, "Equipment.Weapon.Type.Beam");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Weapon_Type_Missile, "Equipment.Weapon.Type.Missile");

UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon, "Equipment.Slot.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Primary, "Equipment.Slot.Weapon.Primary");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Barrel, "Equipment.Slot.Weapon.Barrel");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Mag, "Equipment.Slot.Weapon.Mag");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Scope, "Equipment.Slot.Weapon.Scope");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Stock, "Equipment.Slot.Weapon.Stock");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot_Weapon_Trigger, "Equipment.Slot.Weapon.Trigger");

/*~ Damage Type Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType, "DamageType");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Kinetic, "DamageType.Kinetic");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Plasma, "DamageType.Plasma");
UE_DEFINE_GAMEPLAY_TAG(TAG_DamageType_Explosion, "DamageType.Explosion");

/*~ Damage Property Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage, "Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Dodgeable, "Damage.Dodgeable");

/*~ Character States ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_State, "State");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Sprint, "State.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Crouch, "State.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Aiming, "State.Aiming");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Hit, "State.Hit");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Invincible, "State.Invincible");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dodging, "State.Dodging");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_BlockMovementInput, "State.BlockMovementInput");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_CannotFire, "State.Weapon.CannotFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair, "State.Weapon.Crosshair");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Crosshair_Rifle, "State.Weapon.Crosshair.Rifle");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_UI, "State.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_UI_HideCrosshair,"State.UI.HideCrosshair");

/*~ Event Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Event, "Event");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Death, "Event.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_HitReact, "Event.HitReact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Attack, "Event.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Damage, "Event.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Room, "Event.Room");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Room_Clear, "Event.Room.Clear");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Dash, "Event.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_JustDodge, "Event.JustDodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Hit, "Event.Hit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_HitGhost, "Event.HitGhost");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Kill,"Event.Kill");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Kill_Common,"Event.Kill.Common");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Kill_Elite,"Event.Kill.Elite");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Kill_Boss,"Event.Kill.Boss");

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

UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Reload, "Montage.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Reload_Bullet, "Montage.Reload.Bullet");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Reload_Beam, "Montage.Reload.Beam");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Reload_Missile, "Montage.Reload.Missile");

/*~ Montage Tags - Dash ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Dash, "Montage.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Dash_Front, "Montage.Dash.Front");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Dash_Right, "Montage.Dash.Right");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Dash_Back, "Montage.Dash.Back");
UE_DEFINE_GAMEPLAY_TAG(TAG_Montage_Dash_Left, "Montage.Dash.Left");

/*~ Ability Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability, "Ability");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack, "Ability.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Die, "Ability.Die");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Reload, "Ability.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Dash, "Ability.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_JustDodge, "Ability.JustDodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_SlowMotion, "Ability.SlowMotion");
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

/*~ Target Tags ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_Target, "Target");
UE_DEFINE_GAMEPLAY_TAG(TAG_Target_Lockable, "Target.Lockable");

/*~ GameplayCue Tags - Weapon Impact ~*/
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Weapon_Impact, "GameplayCue.Weapon.Impact");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Weapon_Impact_Bullet, "GameplayCue.Weapon.Impact.Bullet");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Weapon_Impact_Beam, "GameplayCue.Weapon.Impact.Beam");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Weapon_Impact_Missile, "GameplayCue.Weapon.Impact.Missile");