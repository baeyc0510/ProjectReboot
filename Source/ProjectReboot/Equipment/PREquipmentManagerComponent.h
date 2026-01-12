// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PREquipmentManagerComponent.generated.h"


class UEquipmentInstance;
class URogueliteActionData;
class UPREquipActionData;

USTRUCT()
struct FEquipmentSlotItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPREquipActionData> EquipActionData = nullptr;
};

USTRUCT()
struct FEquipmentSlots
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FGameplayTag, FEquipmentSlotItem> SlotItems;
	
	const FEquipmentSlotItem* FindSlotItem(FGameplayTag SlotTag) const
	{
		return SlotItems.Find(SlotTag);
	}
};

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

	/*~ RogueliteSubsystem 이벤트 핸들러 ~*/
	UFUNCTION()
	void HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleActionStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleRunEnded(bool bCompleted);
	
private:
	void BindToRogueliteSubsystem();
	void UnbindFromRogueliteSubsystem();
	
private:
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	FGameplayTagContainer EquipmentActionTags;
	
	FDelegateHandle ActionAcquiredDelegateHandle;
	FDelegateHandle ActionRemovedDelegateHandle;
	FDelegateHandle ActionStackChangedDelegateHandle;
	
	bool bIsBoundToSubsystem = false;
};