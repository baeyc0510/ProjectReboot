// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRActionListItemWidget.generated.h"

class UBorder;
class URogueliteActionData;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionItemClicked, UPRActionListItemWidget*, ClickedItem);

/**
 * 액션 목록 아이템 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRActionListItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯 초기화
	UFUNCTION(BlueprintCallable, Category = "ActionList")
	void InitWidget(URogueliteActionData* InActionData);

	// 선택 상태 설정
	UFUNCTION(BlueprintCallable, Category = "ActionList")
	void SetSelected(bool bInSelected);

	// 선택 여부 반환
	UFUNCTION(BlueprintPure, Category = "ActionList")
	bool IsItemSelected() const { return bIsSelected; }

	// ActionData 반환
	UFUNCTION(BlueprintPure, Category = "ActionList")
	URogueliteActionData* GetActionData() const { return ActionData; }

	// 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "ActionList")
	FOnActionItemClicked OnClicked;

protected:
	/*~ UUserWidget Interface ~*/
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> Border_Background;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_ActionName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	// 기본 Border 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionList|Style")
	FLinearColor NormalBorderColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.8f);

	// 선택 시 Border 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionList|Style")
	FLinearColor SelectedBorderColor = FLinearColor(0.3f, 0.8f, 1.f, 0.8f);

private:
	UPROPERTY()
	TObjectPtr<URogueliteActionData> ActionData;

	bool bIsSelected = false;
};