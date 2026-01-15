// Fill out your copyright notice in the Description page of Project Settings.


#include "PRInputConfig.h"

const UInputAction* UPRInputConfig::FindInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FInputActionMapping& Mapping : InputMappings)
	{
		if (Mapping.InputTag.MatchesTagExact(InputTag) && Mapping.InputAction)
		{
			return Mapping.InputAction;
		}
	}
	return nullptr;
}

FGameplayTag UPRInputConfig::FindTagForInputAction(const UInputAction* InputAction) const
{
	for (const FInputActionMapping& Mapping : InputMappings)
	{
		if (Mapping.InputAction == InputAction)
		{
			return Mapping.InputTag;
		}
	}
	return FGameplayTag();
}