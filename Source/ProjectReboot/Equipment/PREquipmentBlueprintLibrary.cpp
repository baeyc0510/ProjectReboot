// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentBlueprintLibrary.h"
#include "EquipmentInstance.h"
#include "PREquipmentInterface.h"
#include "PREquipmentManagerComponent.h"
#include "PREquipmentTypes.h"
#include "RogueliteBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"


UPREquipmentManagerComponent* UPREquipmentBlueprintLibrary::GetEquipmentManager(AActor* Owner)
{
	if (IPREquipmentInterface* EI = Cast<IPREquipmentInterface>(Owner))
	{
		return EI->GetEquipmentManager();
	}
	return nullptr;
}

void UPREquipmentBlueprintLibrary::SyncEquipmentManager(AActor* From, AActor* To)
{
	if (!IsValid(From) || !IsValid(To))
	{
		return;
	}
	
	UPREquipmentManagerComponent* FromComp = GetEquipmentManager(From);
	UPREquipmentManagerComponent* ToComp = GetEquipmentManager(To);
	
	if (!FromComp || !ToComp)
	{
		return;
	}
	
	ToComp->UnequipAll();
	
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

bool UPREquipmentBlueprintLibrary::TryEquipAction(AActor* Target, UPREquipActionData* ActionData)
{
	if (!IsValid(Target) || !IsValid(ActionData))
	{
		return false;
	}
	
	UPREquipmentManagerComponent* EquipmentManager =GetEquipmentManager(Target);
	if (!EquipmentManager)
	{
		return false;
	}
	
	if (EquipmentManager->HasEquipment(ActionData->EquipmentSlot))
	{
		return false;
	}
	
	EquipmentManager->Equip(ActionData);
	
	return true;
}

void UPREquipmentBlueprintLibrary::OverrideEquipAction(AActor* Target, UPREquipActionData* ActionData)
{
	if (!IsValid(Target) || !IsValid(ActionData))
	{
		return;
	}
	
	UPREquipmentManagerComponent* EquipmentManager = GetEquipmentManager(Target);
	if (!EquipmentManager)
	{
		return;
	}
	
	FGameplayTag SlotToEquip = ActionData->EquipmentSlot;
	TArray<UPREquipActionData*> PartActions;
	PartActions.Add(ActionData);
	
	if (EquipmentManager->HasEquipment(SlotToEquip))
	{
		if (EquipmentManager->IsParentEquipmentSlot(SlotToEquip))
		{
			if (UEquipmentInstance* EquipmentInstance = EquipmentManager->GetEquipmentInstance(ActionData->EquipmentSlot))
			{
				PartActions.Append(EquipmentInstance->GetChildPartActions());
			}
		}
		EquipmentManager->Unequip(SlotToEquip, false);
	}
	
	for (UPREquipActionData* PartAction : PartActions)
	{
		EquipmentManager->Equip(PartAction, false);
	}
	
	// Visual 업데이트는 마지막에 한번 진행
	if (UEquipmentInstance* EquipmentInstance = EquipmentManager->GetEquipmentInstance(ActionData->EquipmentSlot))
	{
		EquipmentInstance->RefreshVisuals();
	}
}

void UPREquipmentBlueprintLibrary::UnequipAction(AActor* Target, UPREquipActionData* ActionData)
{
	if (!IsValid(Target) || !IsValid(ActionData))
	{
		return;
	}
	
	UPREquipmentManagerComponent* EquipmentManager = GetEquipmentManager(Target);
	if (!EquipmentManager)
	{
		return;
	}
	
	EquipmentManager->UnequipByAction(ActionData);
}

void UPREquipmentBlueprintLibrary::UnequipSlot(AActor* Target, FGameplayTag& SlotTag)
{
	if (!IsValid(Target) || !SlotTag.IsValid())
	{
		return;
	}
	
	UPREquipmentManagerComponent* EquipmentManager = GetEquipmentManager(Target);
	if (!EquipmentManager)
	{
		return;
	}
	
	EquipmentManager->Unequip(SlotTag);
}

void UPREquipmentBlueprintLibrary::RemoveEquipmentInstance(AActor* Target, FGameplayTag SlotTag)
{
	if (!IsValid(Target) || !SlotTag.IsValid())
	{
		return;
	}

	UPREquipmentManagerComponent* EquipmentManager = GetEquipmentManager(Target);
	if (!EquipmentManager)
	{
		return;
	}

	UEquipmentInstance* EquipmentInstance = EquipmentManager->GetEquipmentInstance(SlotTag);
	if (!EquipmentInstance)
	{
		return;
	}

	TArray<UPREquipActionData*> AllParts = EquipmentInstance->GetAllAttachedActions();

	for (UPREquipActionData* PartAction : AllParts)
	{
		URogueliteBlueprintLibrary::RemoveAction(Target, PartAction);
	}
}

TArray<UPREquipActionData*> UPREquipmentBlueprintLibrary::GetChangedMeshSpawnInfoActions(
	const TMap<UPREquipActionData*, FSpawnedVisualEntry>& OldMap,
	const TMap<UPREquipActionData*, FSpawnedVisualEntry>& NewMap)
{
	TArray<UPREquipActionData*> ChangedActions;

	for (const auto& NewPair : NewMap)
	{
		UPREquipActionData* ActionData = NewPair.Key;
		const FSpawnedVisualEntry& NewEntry = NewPair.Value;

		// OldMap에서 동일한 ActionData 찾기
		const FSpawnedVisualEntry* OldEntry = OldMap.Find(ActionData);

		// OldMap에 없거나 MeshSpawnInfo가 다르면 변경된 것으로 간주
		if (!OldEntry || !(OldEntry->UsedSpawnInfo == NewEntry.UsedSpawnInfo))
		{
			ChangedActions.Add(ActionData);
		}
	}

	return ChangedActions;
}

