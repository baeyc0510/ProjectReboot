// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "PRCharacterBase.generated.h"

class UPRCommonAttributeSet;
class UPRAbilitySystemComponent;

UCLASS()
class PROJECTREBOOT_API APRCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APRCharacterBase();
	
	/*~ IAbilitySystemInterface ~*/
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/*~ APRCharacterBase Interface ~*/
	float GetBaseMovementSpeed() const {return BaseMovementSpeed;}

protected:
	/** MovementConfigs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BaseMovementSpeed = 500.f;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRAbilitySystemComponent* AbilitySystem;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRCommonAttributeSet* CommonAttributeSet;
};
