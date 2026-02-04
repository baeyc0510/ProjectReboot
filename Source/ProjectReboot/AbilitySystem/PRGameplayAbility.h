// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "PRGameplayAbility.generated.h"

class UAnimMontage;

/**
 * 
 */

UENUM()
enum class EPRAbilityActivationPolicy
{
	OnInputTriggered,
	WhileInputHeld,
	OnGiven,
};

UCLASS()
class PROJECTREBOOT_API UPRGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	// 태그로 몽타주 찾기 공통 헬퍼
	UAnimMontage* FindMontageByGameplayTag(const FGameplayTag& MontageTag) const;

	EPRAbilityActivationPolicy GetActivationPolicy() const {return ActivationPolicy;}
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EPRAbilityActivationPolicy ActivationPolicy;
};
