// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRActionDecisionPanel.generated.h"

class URogueliteActionData;
class UPREquipActionData;
class UVerticalBox;
class UPRActionListItemWidget;
class UPRActorPreviewPanel;

/**
 * 액션 선택 패널 - 목록에서 액션을 선택하고 프리뷰 표시
 */
UCLASS()
class PROJECTREBOOT_API UPRActionDecisionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// 소스 액터 설정 (장비 동기화 대상)
	UFUNCTION(BlueprintCallable, Category = "ActionDecision")
	void SetSourceActor(AActor* InActor, TSubclassOf<AActor> PreviewActorClass);

	// 위젯 새로고침
	UFUNCTION(BlueprintCallable, Category = "ActionDecision")
	void RefreshWidget();

	// 액션 목록 설정 및 아이템 위젯 생성
	UFUNCTION(BlueprintCallable, Category = "ActionDecision")
	void SetDecisionList(const TArray<URogueliteActionData*>& ActionList);

	// 목록 초기화
	UFUNCTION(BlueprintCallable, Category = "ActionDecision")
	void ClearDecisionList();

	// 현재 선택된 아이템 반환
	UFUNCTION(BlueprintPure, Category = "ActionDecision")
	UPRActionListItemWidget* GetSelectedItem() const { return SelectedItem; }

	// 현재 선택된 ActionData 반환
	UFUNCTION(BlueprintPure, Category = "ActionDecision")
	URogueliteActionData* GetSelectedActionData() const;

protected:
	// 아이템 클릭 핸들러
	UFUNCTION()
	void HandleItemClicked(UPRActionListItemWidget* ClickedItem);

	// 아이템 선택 처리
	void SelectItem(UPRActionListItemWidget* Item);

	// 아이템 선택 해제 처리
	void DeselectItem(UPRActionListItemWidget* Item);

	// EquipAction 처리
	void HandleEquipAction(UPREquipActionData* EquipAction, bool bEquip);

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPRActorPreviewPanel> ActorPreviewPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> DecisionListBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDecision")
	TSubclassOf<UPRActionListItemWidget> DecisionItemWidgetClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRActionListItemWidget> SelectedItem;

	UPROPERTY()
	TArray<TObjectPtr<UPRActionListItemWidget>> ItemWidgets;
};
