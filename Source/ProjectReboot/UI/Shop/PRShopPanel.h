// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "PRShopPanel.generated.h"

class UButton;
class UPanelWidget;
class UTextBlock;
class UPRShopComponent;
class UPRShopListItem;

/**
 * 상점 패널 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRShopPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interface ~*/
	// 위젯 초기화
	virtual void NativeConstruct() override;
	// 위젯 종료 처리
	virtual void NativeDestruct() override;

public:
	// ShopComponent 설정 및 바인딩
	UFUNCTION(BlueprintCallable, Category = "Shop|UI")
	void SetShopComponent(UPRShopComponent* InComponent);

protected:
	// 상점 목록 갱신
	void RefreshShopList();

	// 상점 목록 표시 갱신
	void UpdateShopListDisplay();

	// 상점 목록 정리
	void ClearShopList();

	// ShopComponent 바인딩
	void BindToShopComponent();

	// ShopComponent 바인딩 해제
	void UnbindFromShopComponent();

	// 닫기 버튼 클릭 처리
	UFUNCTION()
	void HandleCloseButtonClicked();

	// 아이템 눌림 처리
	UFUNCTION()
	void HandleItemPressed(int32 ItemIndex);

	// 아이템 홀드 완료 처리 (구매)
	UFUNCTION()
	void HandleItemHoldCompleted(int32 ItemIndex);

	// 상점 인벤토리 갱신 처리
	UFUNCTION()
	void HandleShopInventoryUpdated();

	// 잔액 표시 갱신
	void UpdateBalanceDisplay();

protected:
	/*~ Widgets ~*/

	// 닫기 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	// 잔액 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BalanceText;

	// 상품 목록 패널
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ShopListBox;

	// 표시할 화폐 태그
	UPROPERTY(EditDefaultsOnly, Category = "Shop|UI")
	FGameplayTag CurrencyTag = TAG_Currency_Core;

	// 리스트 아이템 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Shop|UI")
	TSubclassOf<UPRShopListItem> ShopListItemClass;

private:
	// 바인딩된 ShopComponent
	TWeakObjectPtr<UPRShopComponent> BoundShopComponent;

	// 생성된 리스트 아이템
	UPROPERTY()
	TArray<TObjectPtr<UPRShopListItem>> ShopListItems;
};
