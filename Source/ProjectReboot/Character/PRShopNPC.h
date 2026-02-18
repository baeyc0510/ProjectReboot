// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Camera/PRActorFocusSubsystem.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "PRShopNPC.generated.h"

class UPRShopComponent;
class UPRShopPanel;
class UUserWidget;

/**
 * 상점 NPC
 */
UCLASS()
class PROJECTREBOOT_API APRShopNPC : public ACharacter, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	APRShopNPC(const FObjectInitializer& ObjectInitializer);
	
	/*~ IPRInteractableInterface ~*/
	// 상호작용 가능 여부
	virtual bool CanInteract(APawn* Interactor) const override;

	// 상호작용 실행
	virtual void Interact(APawn* Interactor) override;

	// 상호작용 UI 텍스트 반환
	virtual FText GetInteractionText() const override;

	// 상호작용 UI 정보 반환
	virtual void GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const override;

protected:
	// 상점 UI 열기
	void OpenShopUI(const APawn* Interactor);

	// 상점 UI 닫힘 처리
	void HandleShopPanelDestruct(UUserWidget* DestructedWidget);

protected:
	// 상호작용 텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FText InteractionDisplayText = NSLOCTEXT("Interaction", "Shop", "상점");

	// 상점 UI 포커스 파라미터
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Focus")
	FActorFocusParams FocusParams;

	// 상점 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UPRShopPanel> ShopPanelClass;

	// 상점 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UPRShopComponent> ShopComponent;

private:
	TWeakObjectPtr<const APawn> InteractorPawn;
};
