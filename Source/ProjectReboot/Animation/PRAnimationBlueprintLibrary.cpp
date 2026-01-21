// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAnimationBlueprintLibrary.h"

#include "PRAnimRegistryInterface.h"

UAnimMontage* UPRAnimationBlueprintLibrary::FindAnimMontageByGameplayTag(AActor* TargetActor, const FGameplayTag& TargetTag)
{
	if (IPRAnimRegistryInterface* ARI = Cast<IPRAnimRegistryInterface>(TargetActor))
	{
		return ARI->FindMontageByGameplayTag(TargetTag);
	}
	
	return nullptr;
}
