// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PRGameplayAbility.generated.h"

/**
 * 
 */

UENUM()
enum class EPRAbilityActivationPolicy
{
	None,
	OnInputTriggered,
	WhileInputHeld,
	OnGiven,
};

UCLASS()
class PROJECTREBOOT_API UPRGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	EPRAbilityActivationPolicy GetActivationPolicy() const {return ActivationPolicy;}
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EPRAbilityActivationPolicy ActivationPolicy;
};
