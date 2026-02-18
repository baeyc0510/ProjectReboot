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

	bool IsValid() const { return Style != nullptr; }
};

UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCrosshairConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshairs")
	TMap<FGameplayTag, FPRCrosshairSetting> CrosshairSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshairs")
	FPRCrosshairSetting DefaultCrosshair;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	const FPRCrosshairSetting& GetCrosshairSetting(const FGameplayTag& CrosshairTag) const;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	bool HasCrosshair(const FGameplayTag& CrosshairTag) const;
};