// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayAbility.h"

#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"

UAnimMontage* UPRGameplayAbility::FindMontageByGameplayTag(const FGameplayTag& MontageTag) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	if (IPRAnimRegistryInterface* AnimRegistry = Cast<IPRAnimRegistryInterface>(AvatarActor))
	{
		return AnimRegistry->FindMontageByGameplayTag(MontageTag);
	}

	return nullptr;
}
