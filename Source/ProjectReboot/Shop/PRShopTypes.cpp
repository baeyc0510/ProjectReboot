// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopTypes.h"
#include "RogueliteActionData.h"

FText FPRShopItemInfo::GetDisplayName() const
{
	return ActionData ? ActionData->DisplayName : FText::GetEmpty();
}

FText FPRShopItemInfo::GetDescription() const
{
	return ActionData ? ActionData->GetFormattedTextWithValuesByIndex(ActionData->Description) : FText::GetEmpty();
}

UTexture2D* FPRShopItemInfo::GetIcon() const
{
	return ActionData ? ActionData->Icon : nullptr;
}
