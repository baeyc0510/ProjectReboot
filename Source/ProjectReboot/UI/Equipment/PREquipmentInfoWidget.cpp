// Fill out your copyright notice in the Description page of Project Settings.

#include "PREquipmentInfoWidget.h"
#include "Components/TextBlock.h"
#include "ProjectReboot/Equipment/PREquipmentActionSet.h"

void UPREquipmentInfoWidget::SetEquipmentInfo(const UPREquipmentActionSet* EquipmentActionSet)
{
	if (!IsValid(EquipmentActionSet))
	{
		return;
	}

	SetDisplayName(EquipmentActionSet->DisplayName);
	SetDescription(EquipmentActionSet->Description);
}

void UPREquipmentInfoWidget::SetDisplayName(const FText& InName)
{
	if (IsValid(NameText))
	{
		NameText->SetText(InName);
	}
}

void UPREquipmentInfoWidget::SetDescription(const FText& InDescription)
{
	if (IsValid(DescriptionText))
	{
		DescriptionText->SetText(InDescription);
	}
}
