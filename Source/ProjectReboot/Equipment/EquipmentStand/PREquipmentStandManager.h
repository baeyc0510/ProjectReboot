// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "PREquipmentStandManager.generated.h"

class UPREquipmentActionSet;
class APREquipmentStand;
class UPREquipActionData;

/**
 * 여러 장비 거치대를 그룹으로 관리하는 매니저 액터
 */
UCLASS()
class PROJECTREBOOT_API APREquipmentStandManager : public AActor
{
	GENERATED_BODY()

public:
	APREquipmentStandManager();

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

public:
	/*~ APREquipmentStandManager Interface ~*/
	// 특정 거치대 선택 처리
	UFUNCTION(BlueprintCallable, Category = "Equipment Selection")
	void SelectStand(APREquipmentStand* Stand, APawn* PlayerPawn);

	// 현재 선택 해제
	UFUNCTION(BlueprintCallable, Category = "Equipment Selection")
	void DeselectCurrentStand();

private:
	// 거치대들의 소속 매니저 설정
	void InitializeStands();

	// 플레이어에게 장비 장착
	void EquipPartsToActor(APawn* Target, const UPREquipmentActionSet* EquipmentActionSet);

	// 플레이어의 기존 장비 해제
	void UnequipFromActor(APawn* Target, const UPREquipmentActionSet* EquipmentActionSet);
	
public:
	// 관리하는 거치대 목록 (레벨 디자이너가 설정)
	UPROPERTY(EditAnywhere, Category = "Equipment Stands")
	TArray<TObjectPtr<APREquipmentStand>> ManagedStands;
	
	// 현재 선택된 거치대 (하나만 선택 가능)
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<APREquipmentStand> CurrentSelectedStand;
};
