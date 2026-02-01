// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PREquipmentInfoWidget.generated.h"

class UTextBlock;
class UPREquipmentActionSet;

/**
 * 장비 세트 정보 표시 위젯
 * - DisplayName과 Description을 표시
 * - 빌보드 위젯 컴포넌트에서 사용
 */
UCLASS()
class PROJECTREBOOT_API UPREquipmentInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 장비 정보 설정
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipmentInfo(const UPREquipmentActionSet* EquipmentActionSet);

	// 장비 이름 설정
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetDisplayName(const FText& InName);

	// 장비 설명 설정
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetDescription(const FText& InDescription);

protected:
	// 장비 이름 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	// 장비 설명 텍스트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;
};
