// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCrosshairConfig.h"

const FPRCrosshairSetting& UPRCrosshairConfig::GetCrosshairSetting(const FGameplayTag& CrosshairTag) const
{
	if (const FPRCrosshairSetting* Found = CrosshairSettings.Find(CrosshairTag))
	{
		return *Found;
	}

	return DefaultCrosshair;
}

bool UPRCrosshairConfig::HasCrosshair(const FGameplayTag& CrosshairTag) const
{
	return CrosshairSettings.Contains(CrosshairTag);
}
