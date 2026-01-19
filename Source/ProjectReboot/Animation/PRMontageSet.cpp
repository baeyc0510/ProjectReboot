// Fill out your copyright notice in the Description page of Project Settings.


#include "PRMontageSet.h"

UAnimMontage* UPRMontageSet::FindMontageByTag(const FGameplayTag& MontageTag) const
{
	if (const FPRMontageVariation* Variation = TagMontageMap.Find(MontageTag))
	{
		return Variation->GetRandomMontage();
	}

	return nullptr;
}
