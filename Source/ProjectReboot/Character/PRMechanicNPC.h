// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "PRMechanicNPC.generated.h"

class APRPlayerCharacter;
class UPRUpgradeModuleData;
class UUserWidget;

/**
 * 정비 NPC
 */
UCLASS()
class PROJECTREBOOT_API APRMechanicNPC : public ACharacter, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	/*~ IPRInteractableInterface ~*/
	// 상호작용 가능 여부
	virtual bool CanInteract(APawn* Interactor) const override;

	// 상호작용 실행
	virtual void Interact(APawn* Interactor) override;

	// 상호작용 UI 텍스트 반환
	virtual FText GetInteractionText() const override;

protected:
	// 업그레이드 UI 열기
	void OpenUpgradeUI(const APawn* Interactor);

protected:
	// 상호작용 텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FText InteractionDisplayText = NSLOCTEXT("Interaction", "Mechanic", "정비");

	// 판매 업그레이드 목록
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TArray<TObjectPtr<UPRUpgradeModuleData>> AvailableUpgrades;

	// 업그레이드 UI 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UUserWidget> UpgradePanelClass;
};
