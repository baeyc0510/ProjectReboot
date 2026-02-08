// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/Camera/PRActorFocusSubsystem.h"
#include "PRActionDecisionPanel.generated.h"

class URogueliteActionData;
class UPREquipActionData;
class UVerticalBox;
class UPRActionListItemWidget;
class UPRActorPreviewPanel;
class UButton;
class UPREquipmentManagerComponent;
struct FActorFocusParams;

// 액션 선택 확정 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionConfirmedSignature, URogueliteActionData*, SelectedAction);

/**
 * 액션 선택 패널 - 목록에서 액션을 선택하고 프리뷰 표시
 */
UCLASS()
class PROJECTREBOOT_API UPRActionDecisionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPRActionDecisionPanel(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 소스 액터 설정 (장비 동기화 대상)
	UFUNCTION(BlueprintCallable, Category = "ActionDecision")
	void SetSourceActor(AActor* InActor);

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

	// 액션 선택 확정 이벤트
	UPROPERTY(BlueprintAssignable, Category = "ActionDecision")
	FOnActionConfirmedSignature OnActionConfirmed;

protected:
	// 아이템 클릭 핸들러
	UFUNCTION()
	void HandleItemClicked(UPRActionListItemWidget* ClickedItem);

	// 선택 확정 버튼 클릭 핸들러
	UFUNCTION()
	void HandleConfirmButtonClicked();

	// 아이템 선택 처리
	void SelectItem(UPRActionListItemWidget* Item);

	// 아이템 선택 해제 처리
	void DeselectItem(UPRActionListItemWidget* Item);

	// EquipAction 처리
	void HandleEquipAction(UPREquipActionData* EquipAction, bool bEquip);

	// 선택 여부에 따라 버튼 활성화 갱신
	void UpdateConfirmButtonState();

	/*~ 장비 관리 ~*/

	// 원래 장비 상태 저장
	void SaveOriginalEquipmentState();

	// 원래 장비 상태로 복원
	void RestoreOriginalEquipment();

	// 특정 슬롯을 원래 장비로 복원
	void RestoreSlotToOriginal(FGameplayTag SlotTag);

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> DecisionListBox;

	// 선택 확정 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDecision")
	TSubclassOf<UPRActionListItemWidget> DecisionItemWidgetClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRActionListItemWidget> SelectedItem;

	UPROPERTY()
	TArray<TObjectPtr<UPRActionListItemWidget>> ItemWidgets;

	// 액터 포커스 파라미터
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FActorFocusParams FocusParams;

	/*~ 장비 복원 관련 ~*/

	// 원래 장비 상태 (슬롯 -> ActionData)
	UPROPERTY()
	TMap<FGameplayTag, UPREquipActionData*> OriginalEquipmentState;

	// 장비 확정 여부 (true면 NativeDestruct에서 복원하지 않음)
	bool bEquipmentConfirmed = false;
};
