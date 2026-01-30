// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "PREquipmentStand.generated.h"

class UPREquipmentActionSet;
class UPREquipActionData;
class UEquipmentInstance;
class APREquipmentStandManager;
class UPRBillboardWidgetComponent;

/**
 * 상호작용 가능한 장비 거치대 액터
 */
UCLASS()
class PROJECTREBOOT_API APREquipmentStand : public AActor, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	APREquipmentStand();

protected:
	/*~ AActor Interface ~*/
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/*~ IPRInteractableInterface ~*/
	virtual bool CanInteract(APawn* Interactor) const override;
	virtual void Interact(APawn* Interactor) override;
	virtual FText GetInteractionText() const override;
	virtual void OnGainInteractFocus(APawn* Interactor) override;
	virtual void OnLoseInteractFocus(APawn* Interactor) override;

public:
	/*~ APREquipmentStand Interface ~*/
	// 거치대 선택 (장비 장착 + 숨김)
	void Select();

	// 거치대 선택 해제 (다시 표시)
	void Deselect();

	// 장비 부품 목록 반환
	const UPREquipmentActionSet* GetEquipmentActionSet() const { return EquipmentActionSet; }

	// 소속 매니저 설정
	void SetOwningManager(APREquipmentStandManager* Manager);

private:
	// 장비 프리뷰 시각화 초기화
	void InitializePreviewVisuals();

public:
	// 거치대가 보유한 장비 부품 목록 (장착 순서대로)
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TObjectPtr<UPREquipmentActionSet> EquipmentActionSet;

	// 상호작용 가능 여부
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bCanInteract = true;

	// 현재 선택된 상태인지 (숨김 처리 여부)
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSelected = false;

protected:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	// 장비 프리뷰용 부착 대상 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> PreviewAttachComponent;

	// 상호작용 프롬프트 빌보드 위젯
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPRBillboardWidgetComponent> InteractionPromptWidget;

	// 장비 프리뷰 인스턴스 (시각적 표현 담당)
	UPROPERTY(Transient)
	TObjectPtr<UEquipmentInstance> PreviewInstance;

	// 소속 매니저 (자동 설정)
	UPROPERTY(Transient)
	TWeakObjectPtr<APREquipmentStandManager> OwningManager;
};
