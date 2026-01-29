
#include "PRFireRateSecondsMMC.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

struct FPRWeaponFireRateToSecondsCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireRateMultiplier);

	FPRWeaponFireRateToSecondsCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPRWeaponAttributeSet, FireRate, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPRWeaponAttributeSet, FireRateMultiplier, Source, true);
	}
};

static const FPRWeaponFireRateToSecondsCapture& GetWeaponFireRateToSecondsCapture()
{
	static FPRWeaponFireRateToSecondsCapture Capture;
	return Capture;
}

UPRFireRateSecondsMMC::UPRFireRateSecondsMMC()
{
	// FireRate 캡처 등록
	RelevantAttributesToCapture.Add(GetWeaponFireRateToSecondsCapture().FireRateDef);
	RelevantAttributesToCapture.Add(GetWeaponFireRateToSecondsCapture().FireRateMultiplierDef);
}

float UPRFireRateSecondsMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float FireRate = 0.0f;
	GetCapturedAttributeMagnitude(GetWeaponFireRateToSecondsCapture().FireRateDef, Spec, EvaluationParameters, FireRate);
	FireRate = FMath::Max(0.f, FireRate);

	float FireRateMultiplier = 1.0f;
	GetCapturedAttributeMagnitude(GetWeaponFireRateToSecondsCapture().FireRateMultiplierDef, Spec, EvaluationParameters, FireRateMultiplier);
	FireRateMultiplier = FMath::Max(0.f, FireRateMultiplier);

	FireRate *= FireRateMultiplier;
	
	// 발/분 -> 초(발당)
	if (FireRate <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return 60.0f / FireRate;
}
