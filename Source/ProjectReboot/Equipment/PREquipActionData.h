// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueliteGASActionData.h"
#include "ProjectReboot/Equipment/PREquipmentTypes.h"
#include "PREquipActionData.generated.h"

/**
 * 
 */

class UEquipmentInstance;

UCLASS()
class PROJECTREBOOT_API UPREquipActionData : public URogueliteGASActionData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTag EquipmentSlot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTagContainer EquipmentTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	bool bSpawnEquipmentInstance = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (EditCondition = "bSpawnEquipmentInstance"))
	TSubclassOf<UEquipmentInstance> EquipmentInstanceType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FEquipmentVisualSettings EquipmentVisualSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	bool bAttachToParentEquipment = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (EditCondition = "bAttachToParentEquipment"))
	FGameplayTag ParentEquipmentSlot;
};