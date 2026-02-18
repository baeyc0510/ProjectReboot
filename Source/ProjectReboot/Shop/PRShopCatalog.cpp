// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopCatalog.h"
#include "RogueliteActionData.h"

const FPRShopItemInfo* UPRShopCatalog::FindInfoForAction(const URogueliteActionData* InAction) const
{
	if (!IsValid(InAction))
	{
		return nullptr;
	}

	for (const FPRShopItemInfo& Info : Items)
	{
		if (Info.ActionData == InAction)
		{
			return &Info;
		}
	}

	return nullptr;
}
