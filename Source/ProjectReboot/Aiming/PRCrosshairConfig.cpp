// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCrosshairConfig.h"

const FCrosshairSetting& UPRCrosshairConfig::GetCrosshairSetting(const FGameplayTag& CrosshairTag) const
{
	if (const FCrosshairSetting* Found = CrosshairSettings.Find(CrosshairTag))
	{
		return *Found;
	}

	return DefaultCrosshair;
}

bool UPRCrosshairConfig::HasCrosshair(const FGameplayTag& CrosshairTag) const
{
	return CrosshairSettings.Contains(CrosshairTag);
}
