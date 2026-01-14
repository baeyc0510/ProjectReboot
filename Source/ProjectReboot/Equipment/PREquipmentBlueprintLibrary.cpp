// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentBlueprintLibrary.h"

#include "EquipmentInstance.h"
#include "PREquipmentManagerComponent.h"

void UPREquipmentBlueprintLibrary::SyncEquipmentManager(AActor* From, AActor* To)
{
	if (!IsValid(From) || !IsValid(To))
	{
		return;
	}
	
	UPREquipmentManagerComponent* FromComp = From->GetComponentByClass<UPREquipmentManagerComponent>();
	UPREquipmentManagerComponent* ToComp = To->GetComponentByClass<UPREquipmentManagerComponent>();
	
	if (!FromComp || !ToComp)
	{
		return;
	}
	
	auto AllEquipments = FromComp->GetAllEquipmentInstances();
	for (auto Equipment : AllEquipments)
	{
		if (!Equipment)
		{
			continue;
		}

		UPREquipActionData* PrimaryAction = Equipment->GetPrimaryActionData();
		if (!PrimaryAction)
		{
			continue;
		}
		
		ToComp->Equip(PrimaryAction,false);
		for (UPREquipActionData* PartAction : Equipment->GetAllAttachedActions())
		{
			if (!PartAction || PartAction == PrimaryAction)
			{
				continue;
			}
			
			ToComp->Equip(PartAction,false);
		}
	}
	
	ToComp->RefreshAllVisuals();
}
