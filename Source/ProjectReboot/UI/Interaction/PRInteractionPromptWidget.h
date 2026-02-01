// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRInteractionPromptWidget.generated.h"

class UImage;
class UTextBlock;
class UPRInteractionViewModel;

/**
 * 상호작용 프롬프트 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interface ~*/
	// 위젯 초기화
	virtual void NativeConstruct() override;

	// 위젯 종료 처리
	virtual void NativeDestruct() override;

protected:
	// ViewModel 바인딩
	void BindToViewModel();

	// ViewModel 바인딩 해제
	void UnbindFromViewModel();

	// 표시 여부 갱신
	UFUNCTION()
	void HandleVisibilityChanged(bool bVisible);

	// 활성 여부 갱신
	UFUNCTION()
	void HandleEnabledChanged(bool bEnabled);

	// 텍스트/아이콘 갱신
	UFUNCTION()
	void HandleContentChanged(const FText& InText, UTexture2D* InIcon);

	// 전체 갱신
	UFUNCTION()
	void HandleViewModelUpdated();

	// 현재 ViewModel 상태로 갱신
	void RefreshFromViewModel();

protected:
	// 상호작용 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InteractionText;

	// 상호작용 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> InteractionIcon;

private:
	// 바인딩된 ViewModel
	TWeakObjectPtr<UPRInteractionViewModel> BoundViewModel;
};