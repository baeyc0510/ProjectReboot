// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PRCrosshairStyle.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCrosshairStyle : public UDataAsset
{
	GENERATED_BODY()
public:
	// 상단 요소 텍스쳐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TObjectPtr<UTexture2D> TopTexture;

	// 하단 요소 텍스쳐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TObjectPtr<UTexture2D> BottomTexture;

	// 좌측 요소 텍스쳐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TObjectPtr<UTexture2D> LeftTexture;

	// 우측 요소 텍스쳐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TObjectPtr<UTexture2D> RightTexture;

	// 중앙 점 텍스쳐 (선택적)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures", meta = (EditCondition = "bShowCenter"))
	TObjectPtr<UTexture2D> CenterTexture;

	// 요소 크기 (픽셀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Size")
	FVector2D ElementSize = FVector2D(4.0f, 16.0f);

	// 중앙 점 크기 (픽셀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Size")
	FVector2D CenterSize = FVector2D(4.0f, 4.0f);

	// 기본 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor DefaultColor = FLinearColor::White;

	// 적 타겟팅 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor EnemyTargetColor = FLinearColor::Red;

	// 발사 불가 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FLinearColor CannotFireColor = FLinearColor::Gray;

	// 중앙 점 표시 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options")
	bool bShowCenter = true;

	// 외곽선 표시 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options")
	bool bShowOutline = false;

	// 외곽선 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Options", meta = (EditCondition = "bShowOutline"))
	FLinearColor OutlineColor = FLinearColor::Black;
};
