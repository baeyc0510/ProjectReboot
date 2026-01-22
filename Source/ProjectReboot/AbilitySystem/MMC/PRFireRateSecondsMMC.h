#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "PRFireRateSecondsMMC.generated.h"

/**
 * 무기 발사 속도(발/분)를 초 단위로 변환하는 MMC
 */
UCLASS(meta = (DisplayName = "FireRateToSeconds"))
class PROJECTREBOOT_API UPRFireRateSecondsMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRFireRateSecondsMMC();

	/*~ UGameplayModMagnitudeCalculation Interface ~*/

	// 발사 속도(발/분) -> 초 변환
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
