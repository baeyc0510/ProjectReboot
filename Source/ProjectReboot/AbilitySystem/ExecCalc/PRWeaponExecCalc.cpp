// PRWeaponExecCalc.cpp
#include "PRWeaponExecCalc.h"

#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRWeaponExecCalc::UPRWeaponExecCalc()
{
}

void UPRWeaponExecCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_FireRate, UPRWeaponAttributeSet::GetFireRateAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_ReloadTime, UPRWeaponAttributeSet::GetReloadTimeAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_BaseDamage, UPRWeaponAttributeSet::GetBaseDamageAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_DamageMultiplier, UPRWeaponAttributeSet::GetDamageMultiplierAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_Ammo, UPRWeaponAttributeSet::GetAmmoAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_MaxAmmo, UPRWeaponAttributeSet::GetMaxAmmoAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_ReserveAmmo, UPRWeaponAttributeSet::GetReserveAmmoAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_MaxReserveAmmo, UPRWeaponAttributeSet::GetMaxReserveAmmoAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_MaxPenetration, UPRWeaponAttributeSet::GetMaxPenetrationAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_PenetrationDamageFalloff, UPRWeaponAttributeSet::GetPenetrationDamageFalloffAttribute(), OutExecutionOutput);
	TryApplySetByCaller(Spec, TAG_SetByCaller_Weapon_EffectRadius, UPRWeaponAttributeSet::GetEffectRadiusAttribute(), OutExecutionOutput);
}

void UPRWeaponExecCalc::TryApplySetByCaller(const FGameplayEffectSpec& Spec,
	const FGameplayTag& SetByCallerTag,
	const FGameplayAttribute& TargetAttribute,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const float* MagnitudePtr = Spec.SetByCallerTagMagnitudes.Find(SetByCallerTag);
	if (!MagnitudePtr)
	{
		return;
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		TargetAttribute,
		EGameplayModOp::Override,
		*MagnitudePtr));
}
