// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PREquipmentBlueprintLibrary.generated.h"

struct FGameplayTag;
class UPREquipActionData;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPREquipmentBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void SyncEquipmentManager(AActor* From, AActor* To);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static bool TryEquipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void OverrideEquipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void UnequipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void UnequipSlot(AActor* Target, FGameplayTag& SlotTag);
	
};
