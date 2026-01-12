// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PREquipmentManagerComponent.generated.h"


class URogueliteActionData;
class UPRRogueliteEquipActionData;

UCLASS(meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPREquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPREquipmentManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*~ Subsystem 이벤트 핸들러 ~*/

	UFUNCTION()
	void HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleRunEnded(bool bCompleted);
	
	// 슬롯에 장착
	UFUNCTION(BlueprintCallable, Category = "Equipment|Slots")
	bool EquipActionToSlot(UPRRogueliteEquipActionData* Action, FGameplayTag SlotTag);

	// 슬롯에서 해제
	UFUNCTION(BlueprintCallable, Category = "Equipment|Slots")
	void UnequipActionFromSlot(UPRRogueliteEquipActionData* Action, FGameplayTag SlotTag);

	// 슬롯 내용 조회
	UFUNCTION(BlueprintCallable, Category = "Equipment|Slots")
	TArray<UPRRogueliteEquipActionData*> GetSlotContents(FGameplayTag SlotTag) const;

	// 슬롯 사용 개수
	UFUNCTION(BlueprintCallable, Category = "Equipment|Slots")
	int32 GetSlotCount(FGameplayTag SlotTag) const;

	// 슬롯 풀 여부
	UFUNCTION(BlueprintCallable, Category = "Equipment|Slots")
	bool IsSlotFull(FGameplayTag SlotTag, int32 MaxCount) const;
	
private:
	void BindToRogueliteSubsystem();
	void UnbindFromRogueliteSubsystem();
	
	
private:
	bool bIsBoundToSubsystem = false;
};