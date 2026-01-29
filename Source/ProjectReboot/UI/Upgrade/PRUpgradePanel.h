// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRUpgradePanel.generated.h"

class UButton;
class UPanelWidget;
class UPRUpgradeListItem;
class UPRUpgradeModuleData;
class UPRUpgradeViewModel;

/**
 * 업그레이드 패널 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRUpgradePanel : public UUserWidget
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

	// 리스트 아이템 갱신
	void RefreshUpgradeList();

	// 리스트 아이템 표시 갱신
	void UpdateUpgradeListDisplay();

	// 리스트 아이템 정리
	void ClearUpgradeList();

	// 닫기 버튼 클릭 처리
	UFUNCTION()
	void HandleCloseButtonClicked();

	// 아이템 눌림 처리
	UFUNCTION()
	void HandleItemPressed(UPRUpgradeModuleData* InModule);

	// 아이템 홀드 완료 처리
	UFUNCTION()
	void HandleItemHoldCompleted(UPRUpgradeModuleData* InModule);

	// ViewModel 갱신 처리
	UFUNCTION()
	void HandleViewModelUpdated();

protected:
	// 닫기 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	// 업그레이드 목록 패널
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> UpgradeListBox;

	// 리스트 아이템 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade|UI")
	TSubclassOf<UPRUpgradeListItem> UpgradeListItemClass;

private:
	// 바인딩된 ViewModel
	TWeakObjectPtr<UPRUpgradeViewModel> BoundViewModel;

	// 생성된 리스트 아이템
	UPROPERTY()
	TArray<TObjectPtr<UPRUpgradeListItem>> UpgradeListItems;
};
