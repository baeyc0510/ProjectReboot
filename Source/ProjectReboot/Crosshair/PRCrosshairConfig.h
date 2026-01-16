// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PRCrosshairConfig.generated.h"

class UPRCrosshairStyle;
/**
 * 크로스헤어 설정
 */
USTRUCT(BlueprintType)
struct FPRCrosshairSetting
{
	GENERATED_BODY()

	// 스타일 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UPRCrosshairStyle> Style;

	// 기본 스프레드 (픽셀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseSpread = 10.0f;

	// 최대 스프레드 (픽셀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpread = 50.0f;

	// 이동 시 스프레드 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MovementSpreadMultiplier = 2.0f;

	// 사격 시 반동 스프레드 증가량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RecoilSpreadIncrease = 5.0f;

	// 스프레드 회복 속도 (픽셀/초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadRecoveryRate = 30.0f;

	// ADS 시 스프레드 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ADSSpreadMultiplier = 0.5f;

	// ADS 시 크로스헤어 숨김 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bHideOnADS = false;

	bool IsValid() const { return Style != nullptr; }
};

UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCrosshairConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshairs", Meta = (TitleProperty = "Key"))
	TMap<FGameplayTag, FPRCrosshairSetting> CrosshairSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshairs")
	FPRCrosshairSetting DefaultCrosshair;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	const FPRCrosshairSetting& GetCrosshairSetting(const FGameplayTag& CrosshairTag) const;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	bool HasCrosshair(const FGameplayTag& CrosshairTag) const;
};