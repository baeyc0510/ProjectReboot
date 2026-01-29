// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "PRUpgradeListItem.generated.h"

class UButton;
class UImage;
class UOverlay;
class UProgressBar;
class UTextBlock;
class UWidgetAnimation;
class UPRUpgradeModuleData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeItemPressed, UPRUpgradeModuleData*, Module);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeItemHoldCompleted, UPRUpgradeModuleData*, Module);

/**
 * 업그레이드 목록 아이템 위젯
 * - 호버 시 애니메이션 재생
 * - 클릭 유지 시 프로그레스바가 차고 완료되면 이벤트 브로드캐스트
 */
UCLASS()
class PROJECTREBOOT_API UPRUpgradeListItem : public UUserWidget
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

	// 모듈 초기화
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ListItem")
	void InitWidget(UPRUpgradeModuleData* InModule);

	// 모듈 설정
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ListItem")
	void SetModule(UPRUpgradeModuleData* InModule);

	// 모듈 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ListItem")
	UPRUpgradeModuleData* GetModule() const { return Module; }

	// 표시 갱신
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ListItem")
	void UpdateDisplay();

	// 레벨/가격 표시 갱신
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ListItem")
	void UpdateLevelInfo(int32 InCurrentLevel, int32 InMaxLevel, float InNextCost);

public:
	/*~ Events ~*/

	// 아이템 눌림 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeItemPressed OnItemPressed;

	// 홀드 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeItemHoldCompleted OnItemHoldCompleted;

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

	// 레벨 텍스트 갱신
	void RefreshLevelText();

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

	// 홀드 진행 바
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PB_HoldProgress;

	// 현재 레벨 텍스트
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_CurrentLevel;

	// 다음 레벨 텍스트
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_NextLevel;

	// 다음 레벨 비용 텍스트
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_NextCost;

	// 레벨업 오버레이
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> LevelUpOverlay;

	// 호버 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HoverAnimation;

protected:
	// 홀드 완료 시간
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade|Hold")
	float HoldDuration = 0.8f;

	// 홀드 갱신 간격
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade|Hold")
	float HoldTickInterval = 0.02f;

private:
	// 모듈 데이터
	UPROPERTY()
	TObjectPtr<UPRUpgradeModuleData> Module;

	// 홀드 경과 시간
	float HoldElapsed = 0.0f;

	// 홀드 진행 여부
	bool bIsHolding = false;

	// 호버 상태
	bool bIsHovered = false;

	// 타이머 핸들
	FTimerHandle HoldTimerHandle;
	
	// Stats
	int32 CurrentLevel = 0;
	int32 MaxLevel = 0;
	float NextCost = 0.0f;
};
