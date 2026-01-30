// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentActionSet.h"

#include "PREquipActionData.h"
#include "RogueliteSubsystem.h"

void UPREquipmentActionSet::AcquireActionSet(UObject* WorldContextObject) const
{
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(WorldContextObject);
	if (!RogueliteSubsystem)
	{
		return;
	}
	
	if (!PrimaryAction)
	{
		return;
	}
	
	RogueliteSubsystem->AcquireAction(PrimaryAction);
	
	for (UPREquipActionData* PartAction : PartActions)
	{
		if (PartAction)
		{
			RogueliteSubsystem->AcquireAction(PartAction);
		}
	}
}

FGameplayTag UPREquipmentActionSet::GetPrimarySlotTag() const
{
	if (PrimaryAction)
	{
		return PrimaryAction->EquipmentSlot;
	}
	return FGameplayTag::EmptyTag;
}
