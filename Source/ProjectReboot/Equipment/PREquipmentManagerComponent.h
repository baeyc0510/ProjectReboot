// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PREquipmentManagerComponent.generated.h"


class URogueliteSubsystem;
class UEquipmentInstance;
class URogueliteActionData;
class UPREquipActionData;

USTRUCT()
struct FEquipmentSlotEntry
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UEquipmentInstance> Instance = nullptr;

    UPROPERTY()
    TObjectPtr<UPREquipActionData> ActionData = nullptr;

    bool IsValid() const { return ActionData != nullptr; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentChangedDelegate, FGameplayTag, SlotTag, UEquipmentInstance*, Instance, UPREquipActionData*, ActionData);

UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPREquipmentManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPREquipmentManagerComponent();

protected:
    /*~ UActorComponent Interface ~*/
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
public:
    /*~ UPREquipmentManagerComponent Interface ~*/
    /*~ Equipment Operations ~*/
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void Equip(UPREquipActionData* ActionData, bool bRefreshVisuals = true);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void Unequip(FGameplayTag SlotTag, bool bRefreshVisuals = true);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipAll();

    /*~ Queries ~*/
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UEquipmentInstance* GetEquipmentInstance(FGameplayTag SlotTag) const;
    
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    TArray<UEquipmentInstance*> GetAllEquipmentInstances() const;
    
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UPREquipActionData* GetActionData(FGameplayTag SlotTag) const;
    
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool HasEquipment(FGameplayTag SlotTag) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool IsParentEquipmentSlot(FGameplayTag SlotTag) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void RefreshAllVisuals();
public:
    /*~ Delegates ~*/
    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FOnEquipmentChangedDelegate OnEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FOnEquipmentChangedDelegate OnUnequipped;

private:
    // Subsystem Binding
    void BindToRogueliteSubsystem();
    void UnbindFromRogueliteSubsystem();
    void ProcessExistingActions();
    TArray<UPREquipActionData*> GetSortedActionsByDependency(TArray<URogueliteActionData*>& InActions) const;

    // Subsystem Helper
    URogueliteSubsystem* GetRogueliteSubsystem() const;
    
    // Event Handlers
    UFUNCTION()
    void HandleActionAcquired(URogueliteActionData* ActionData, int32 OldStacks, int32 NewStacks);

    UFUNCTION()
    void HandleActionRemoved(URogueliteActionData* ActionData, int32 OldStacks, int32 NewStacks);

    UFUNCTION()
    void HandleActionStackChanged(URogueliteActionData* ActionData, int32 OldStacks, int32 NewStacks);

    UFUNCTION()
    void HandleRunEnded(bool bCompleted);
    
    // Internals
    void HandleActionAcquired_Internal(UPREquipActionData* EquipAction, bool bRefreshVisuals = true);
    void HandleActionRemoved_Internal(UPREquipActionData* EquipAction, bool bRefreshVisuals = true);
    UEquipmentInstance* CreateInstance(UPREquipActionData* ActionData);
    USceneComponent* GetAttachTarget() const;
    void UnequipChildren(FGameplayTag ParentSlotTag);
    TArray<FGameplayTag> FindChildSlots(FGameplayTag ParentSlotTag) const;

protected:
    UPROPERTY(EditDefaultsOnly)
    FGameplayTagContainer ActionTagsToManage;
    
private:
    UPROPERTY()
    TMap<FGameplayTag, FEquipmentSlotEntry> Slots;

    FDelegateHandle ActionAcquiredHandle;
    FDelegateHandle ActionRemovedHandle;
    FDelegateHandle ActionStackChangedHandle;

    bool bIsBoundToSubsystem = false;
};