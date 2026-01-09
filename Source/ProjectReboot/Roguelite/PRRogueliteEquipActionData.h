// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueliteGASActionData.h"
#include "ProjectReboot/Equipment/PREquipmentTypes.h"
#include "PRRogueliteEquipActionData.generated.h"

/**
 * 
 */

UCLASS()
class PROJECTREBOOT_API UPRRogueliteEquipActionData : public URogueliteGASActionData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FEquipmentVisualSettings EquipmentVisualSettings;
};