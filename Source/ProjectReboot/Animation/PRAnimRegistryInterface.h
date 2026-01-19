// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PRAnimRegistryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPRAnimRegistryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTREBOOT_API IPRAnimRegistryInterface
{
	GENERATED_BODY()
	
public:
	virtual UAnimMontage* FindMontageByGameplayTag(const FGameplayTag& MontageTag) const = 0;
};
