// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PREquipmentActionSet.generated.h"

class UPREquipActionData;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPREquipmentActionSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FText Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UPREquipActionData> PrimaryAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<UPREquipActionData*> PartActions;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void AcquireActionSet(UObject* WorldContextObject) const;
	
	UFUNCTION(BlueprintPure, Category = "Equipment")
	FGameplayTag GetPrimarySlotTag() const;
};