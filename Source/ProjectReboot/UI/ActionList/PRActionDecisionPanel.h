// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PRActionDecisionPanel.generated.h"

class URogueliteActionData;
class UPREquipActionData;
class UVerticalBox;
class UPRActionListItemWidget;
class UPRActorPreviewPanel;
class UButton;
class ACameraActor;
class UPREquipmentManagerComponent;

/**
 * 액션 선택 패널 - 목록에서 액션을 선택하고 프리뷰 표시
 */
UCLASS()
class PROJECTREBOOT_API UPRActionDecisionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
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

	/*~ 카메라 관리 ~*/

	// 프리뷰 카메라 설정
	void SetupPreviewCamera();

	// 프리뷰 카메라 활성화
	void ActivatePreviewCamera();

	// 원래 카메라로 복원
	void RestoreCamera();

	// 프리뷰 카메라 정리
	void CleanupPreviewCamera(bool bImmediate = false);

	/*~ 장비 관리 ~*/

	// 원래 장비 상태 저장
	void SaveOriginalEquipmentState();

	// 원래 장비 상태로 복원
	void RestoreOriginalEquipment();

	// 특정 슬롯을 원래 장비로 복원
	void RestoreSlotToOriginal(FGameplayTag SlotTag);

	/*~ 플레이어 상태 관리 ~*/

	// 플레이어 상태 잠금 (회전 고정 + 카메라 향해 회전)
	void LockPlayerState();

	// 플레이어 상태 잠금 해제
	void UnlockPlayerState();
	
	// 다른 UI 상태 처리
	void HandleOtherUIVisibility(bool bIsVisible);
	

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

	/*~ 카메라 관련 ~*/

	UPROPERTY()
	TObjectPtr<ACameraActor> PreviewCameraActor;

	UPROPERTY()
	TWeakObjectPtr<AActor> OriginalViewTarget;

	// 플레이어로부터 카메라까지의 거리
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Camera")
	float CameraDistance = 250.0f;

	// 카메라 높이 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Camera")
	float CameraHeightOffset = 60.0f;

	// 정면에서의 측면 각도 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Camera")
	float CameraYawOffset = 25.0f;

	// 카메라 전환 블렌딩 시간
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Camera")
	float BlendTime = 0.4f;

	// 화면상 좌우 오프셋 (양수 = 플레이어 기준 우측로 카메라 이동)
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Camera")
	float ScreenHorizontalOffset = -100.0f;


	/*~ 장비 복원 관련 ~*/

	// 원래 장비 상태 (슬롯 -> ActionData)
	UPROPERTY()
	TMap<FGameplayTag, UPREquipActionData*> OriginalEquipmentState;

	// 장비 확정 여부 (true면 NativeDestruct에서 복원하지 않음)
	bool bEquipmentConfirmed = false;

	/*~ 플레이어 상태 관련 ~*/

	// 원래 캐릭터 회전
	FRotator OriginalActorRotation;

	// 원래 OrientRotationToMovement 설정
	bool bOriginalOrientRotationToMovement = false;
};
