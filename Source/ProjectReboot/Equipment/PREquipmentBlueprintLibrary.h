// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PREquipmentBlueprintLibrary.generated.h"

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
};
