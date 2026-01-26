// PRDamageExecCalc.cpp
#include "PRDamageExecCalc.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRDamageExecCalc::UPRDamageExecCalc()
{
}

void UPRDamageExecCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 태그 수집
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);

	FGameplayTagContainer AllTags = AssetTags;
	AllTags.AppendTags(GrantedTags);

	// 무기/데미지 타입 추출
	FGameplayTag WeaponTypeTag;
	GetWeaponTypeTag(AllTags, WeaponTypeTag);

	FGameplayTag DamageTypeTag;
	GetDamageTypeTag(AllTags, DamageTypeTag);
	
	// 기본 데미지
	const float BaseDamage = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, false, 0.0f);
	if (BaseDamage <= 0.0f)
	{
		return;
	}

	// 관통 카운트/감쇄 (키가 있을 때만)
	float PenetrationCount = 0.0f;
	if (const float* PenetrationCountPtr = Spec.SetByCallerTagMagnitudes.Find(TAG_SetByCaller_Combat_PenetrationCount))
	{
		PenetrationCount = FMath::Max(0.0f, *PenetrationCountPtr);
	}

	float PenetrationDamageFalloff = 1.0f;
	if (const float* PenetrationFalloffPtr = Spec.SetByCallerTagMagnitudes.Find(TAG_SetByCaller_Weapon_PenetrationDamageFalloff))
	{
		PenetrationDamageFalloff = FMath::Clamp(*PenetrationFalloffPtr, 0.0f, 1.0f);
	}

	// 최종 데미지 계산
	const float WeaponTypeMultiplier = GetWeaponTypeMultiplier(WeaponTypeTag);
	const float DamageTypeMultiplier = GetDamageTypeMultiplier(DamageTypeTag);
	float FinalDamage = BaseDamage * WeaponTypeMultiplier * DamageTypeMultiplier;

	if (!FMath::IsNearlyEqual(PenetrationDamageFalloff, 1.0f) && PenetrationCount > 0.0f)
	{
		FinalDamage *= FMath::Pow(PenetrationDamageFalloff, PenetrationCount);
	}

	if (FinalDamage <= 0.0f)
	{
		return;
	}

	// IncomingDamage 메타 어트리뷰트에 출력 (실제 적용은 AttributeSet에서 처리)
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPRCommonAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage));
}

bool UPRDamageExecCalc::GetWeaponTypeTag(const FGameplayTagContainer& InTags, FGameplayTag& OutWeaponTypeTag) const
{
	for (auto& Tag : InTags)
	{
		if (Tag.MatchesTag(TAG_Equipment_Weapon_Type))
		{
			OutWeaponTypeTag = Tag;
			return true;
		}
	}

	return false;
}

bool UPRDamageExecCalc::GetDamageTypeTag(const FGameplayTagContainer& InTags, FGameplayTag& OutDamageTypeTag) const
{
	for (auto& Tag : InTags)
	{
		if (Tag.MatchesTag(TAG_DamageType))
		{
			OutDamageTypeTag = Tag;
			return true;
		}
	}

	return false;
}

float UPRDamageExecCalc::GetWeaponTypeMultiplier(const FGameplayTag& WeaponTypeTag) const
{
	const float* Multiplier = WeaponTypeMultipliers.Find(WeaponTypeTag);
	return Multiplier ? *Multiplier : 1.0f;
}

float UPRDamageExecCalc::GetDamageTypeMultiplier(const FGameplayTag& DamageTypeTag) const
{
	const float* Multiplier = DamageTypeMultipliers.Find(DamageTypeTag);
	return Multiplier ? *Multiplier : 1.0f;
}

float UPRDamageExecCalc::GetShieldDamageMultiplier(const FGameplayTag& WeaponTypeTag) const
{
	const float* Multiplier = DamageTypeShieldMultipliers.Find(WeaponTypeTag);
	return Multiplier ? *Multiplier : 1.0f;
}

float UPRDamageExecCalc::GetHitImmunityDamageMultiplier(const FGameplayTag& DamageTypeTag) const
{
	const float* Multiplier = DamageTypeHitImmunityMultipliers.Find(DamageTypeTag);
	return Multiplier ? *Multiplier : 1.0f;
}
