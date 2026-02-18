// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ProjectReboot/Shop/PRShopTypes.h"
#include "PRShopListItem.generated.h"


class UImage;
class UOverlay;
class UProgressBar;
class UTextBlock;
class UWidgetAnimation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopListItemPressed, int32, ItemIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopListItemHoldCompleted, int32, ItemIndex);

/**
 * 상점 아이템 위젯
 * - 호버 시 애니메이션 재생
 * - 클릭 유지 시 프로그레스바가 차고 완료되면 구매 이벤트 브로드캐스트
 */
UCLASS()
class PROJECTREBOOT_API UPRShopListItem : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interface ~*/
	// 위젯 초기화
	virtual void NativeConstruct() override;
	// 위젯 종료 처리
	virtual void NativeDestruct() override;

public:
	/*~ Initialization ~*/

	// 아이템 초기화
	UFUNCTION(BlueprintCallable, Category = "Shop|ListItem")
	void InitWidget(const FPRShopItemInfo& InItemInfo, int32 InItemIndex);

	// 상태 갱신
	UFUNCTION(BlueprintCallable, Category = "Shop|ListItem")
	void UpdateDisplay(bool bIsSoldOut, bool bCanPurchase);

	// 아이템 인덱스 반환
	UFUNCTION(BlueprintPure, Category = "Shop|ListItem")
	int32 GetItemIndex() const { return ItemIndex; }

public:
	/*~ Events ~*/

	// 아이템 눌림 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopListItemPressed OnItemPressed;

	// 홀드 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopListItemHoldCompleted OnItemHoldCompleted;

protected:
	/*~ Button Events ~*/

	// 버튼 눌림 처리
	UFUNCTION()
	void HandleButtonPressed();

	// 버튼 릴리즈 처리
	UFUNCTION()
	void HandleButtonReleased();

	// 버튼 호버 처리
	UFUNCTION()
	void HandleButtonHovered();

	// 버튼 언호버 처리
	UFUNCTION()
	void HandleButtonUnhovered();

protected:
	/*~ Hold Purchase ~*/

	// 홀드 시작
	void StartHold();

	// 홀드 종료
	void StopHold(bool bResetProgress = true);

	// 홀드 진행 갱신
	void HandleHoldTick();

	// 홀드 완료 처리
	void HandleHoldCompleted();

	// 진행도 UI 갱신
	void UpdateProgressBar(float Progress);

	// 아이템 표시 갱신
	void RefreshDisplay();

protected:
	/*~ Widgets ~*/

	// 구매 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> PurchaseButton;

	// 아이콘
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Icon;

	// 이름
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_Name;

	// 설명
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_Description;

	// 가격 텍스트
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_Price;

	// 홀드 진행 바
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PB_HoldProgress;

	// 판매 완료 오버레이
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> SoldOutOverlay;

	// 호버 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HoverAnimation;

protected:
	// 홀드 완료 시간
	UPROPERTY(EditDefaultsOnly, Category = "Shop|Hold")
	float HoldDuration = 0.8f;

	// 홀드 갱신 간격
	UPROPERTY(EditDefaultsOnly, Category = "Shop|Hold")
	float HoldTickInterval = 0.02f;

private:
	// 캐시된 아이템 정보
	FPRShopItemInfo CachedItemInfo;

	// 아이템 인덱스
	int32 ItemIndex = INDEX_NONE;

	// 홀드 경과 시간
	float HoldElapsed = 0.0f;

	// 홀드 진행 여부
	bool bIsHolding = false;

	// 호버 상태
	bool bIsHovered = false;

	// 판매 완료 여부
	bool bIsSoldOut = false;

	// 타이머 핸들
	FTimerHandle HoldTimerHandle;
};
