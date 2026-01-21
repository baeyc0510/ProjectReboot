// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PRAnimationBlueprintLibrary.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRAnimationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static UAnimMontage* FindAnimMontageByGameplayTag(AActor* TargetActor, const FGameplayTag& TargetTag);
};
