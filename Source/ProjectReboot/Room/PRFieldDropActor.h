// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "PRFieldDropActor.generated.h"

class USphereComponent;
class URogueliteActionData;
class UPRBillboardWidgetComponent;

/**
 * 필드 드랍 액터 - 상호작용 시 액션 즉시 획득
 */
UCLASS()
class PROJECTREBOOT_API APRFieldDropActor : public AActor, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	APRFieldDropActor();

	// 드랍할 액션 데이터 설정 (위젯 텍스트/아이콘도 갱신)
	UFUNCTION(BlueprintCallable, Category = "FieldDrop")
	void SetActionData(URogueliteActionData* InActionData);

	// 드랍할 액션 데이터 조회
	UFUNCTION(BlueprintPure, Category = "FieldDrop")
	URogueliteActionData* GetActionData() const { return ActionData; }

protected:
	/*~ IPRInteractableInterface ~*/
	virtual bool CanInteract(APawn* Interactor) const override;
	virtual void Interact(APawn* Interactor) override;
	virtual FText GetInteractionText() const override;
	virtual void GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const override;
	virtual USoundBase* GetInteractionSound() override { return InteractionSound; }

	// 빌보드 위젯에 액션 정보 반영
	void UpdatePromptWidget();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> RootSphereComponent;
	
	// 상호작용 프롬프트 빌보드 위젯
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPRBillboardWidgetComponent> InteractionBillboardWidget;

	// 드랍할 액션 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldDrop")
	TObjectPtr<URogueliteActionData> ActionData;

	// 상호작용 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TObjectPtr<USoundBase> InteractionSound;

	// 상호작용 가능 여부
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bIsInteractable = true;
};
