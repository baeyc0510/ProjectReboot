// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRBillboardInfoWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 범용 빌보드 정보 위젯 (텍스처 + 텍스트)
 */
UCLASS()
class PROJECTREBOOT_API UPRBillboardInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 텍스트 설정
	UFUNCTION(BlueprintCallable, Category = "BillboardInfo")
	void SetDisplayText(const FText& InText);

	// 아이콘 설정
	UFUNCTION(BlueprintCallable, Category = "BillboardInfo")
	void SetIcon(UTexture2D* InIcon);

	// 텍스트 + 아이콘 일괄 설정
	UFUNCTION(BlueprintCallable, Category = "BillboardInfo")
	void SetContent(const FText& InText, UTexture2D* InIcon);

protected:
	// 표시 텍스트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DisplayText;

	// 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;
};
