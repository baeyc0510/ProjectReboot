// PRCommonExecCalc.cpp
#include "PRCommonExecCalc.h"

#include "GameplayEffectExtension.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRCommonExecCalc::UPRCommonExecCalc()
{
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_MoveSpeed, {UPRCommonAttributeSet::GetMoveSpeedAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_MaxHealth, {UPRCommonAttributeSet::GetMaxHealthAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_Health, {UPRCommonAttributeSet::GetHealthAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_MaxShield, {UPRCommonAttributeSet::GetMaxShieldAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_Shield, {UPRCommonAttributeSet::GetShieldAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_Stagger, {UPRCommonAttributeSet::GetStaggerAttribute()});
	SetByCallerAttributeMap.Add(TAG_SetByCaller_Common_HitImmunity, {UPRCommonAttributeSet::GetHitImmunityAttribute()});
}
