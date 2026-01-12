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
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
public:
    // Equipment Operations
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void Equip(UPREquipActionData* ActionData);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void Unequip(FGameplayTag SlotTag);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipAll();

    // Query
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UEquipmentInstance* GetInstance(FGameplayTag SlotTag) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UPREquipActionData* GetActionData(FGameplayTag SlotTag) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    TArray<UEquipmentInstance*> GetAllInstances() const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool HasEquipment(FGameplayTag SlotTag) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool IsParentEquipmentSlot(FGameplayTag SlotTag) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FOnEquipmentChangedDelegate OnEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FOnEquipmentChangedDelegate OnUnequipped;

private:
    // Subsystem Binding
    void BindToRogueliteSubsystem();
    void UnbindFromRogueliteSubsystem();
    void ProcessExistingActions();

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
    
    // Internal
    UEquipmentInstance* CreateInstance(UPREquipActionData* ActionData);
    USceneComponent* GetAttachTarget() const;
    void RefreshAllVisuals();
    void UnequipChildren(FGameplayTag ParentSlotTag);
    TArray<FGameplayTag> FindChildSlots(FGameplayTag ParentSlotTag) const;

private:
    UPROPERTY()
    TMap<FGameplayTag, FEquipmentSlotEntry> Slots;

    UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
    FGameplayTagContainer ActionTagsToReceive;

    FDelegateHandle ActionAcquiredHandle;
    FDelegateHandle ActionRemovedHandle;
    FDelegateHandle ActionStackChangedHandle;

    bool bIsBoundToSubsystem = false;
};