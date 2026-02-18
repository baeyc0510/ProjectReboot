// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRModalWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 모달 위젯
 * 간단한 텍스트 메시지 표시 및 확인 버튼
 */
UCLASS()
class PROJECTREBOOT_API UPRModalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interface ~*/
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// 모달 텍스트 설정
	UFUNCTION(BlueprintCallable, Category = "UI|Modal")
	void SetText(const FText& InText);

protected:
	// 확인 버튼 클릭 처리
	UFUNCTION()
	void HandleConfirmButtonClicked();

protected:
	// 메시지 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	// 확인 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;
};
