// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"
#include "PRCharacterBase.generated.h"

class UPRMontageSet;
class UPRCommonAttributeSet;
class UPRAbilitySystemComponent;

UCLASS()
class PROJECTREBOOT_API APRCharacterBase : public ACharacter, public IAbilitySystemInterface, public IPRCombatInterface, public IPRAnimRegistryInterface
{
	GENERATED_BODY()

public:
	APRCharacterBase();
	
	/*~ IAbilitySystemInterface ~*/
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/*~ IPRCombatInterface ~*/
	virtual bool IsDead() const override;
	virtual void Die(const FGameplayEffectContextHandle& EffectContext) override;
	virtual void FinishDie() override;
	
	/*~ IPRAnimRegistryInterface Interface ~*/
	virtual UAnimMontage* FindMontageByGameplayTag(const FGameplayTag& MontageTag) const override;
	
	/*~ APRCharacterBase Interface ~*/
	float GetBaseMovementSpeed() const {return BaseMovementSpeed;}
	virtual void HandleCollisionAndMovementOnDeath();
	
protected:
	/** MovementConfigs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BaseMovementSpeed = 500.f;
	
	/** Animations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Animation")
	TObjectPtr<UPRMontageSet> MontageSet;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRAbilitySystemComponent> AbilitySystem;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRCommonAttributeSet> CommonAttributeSet;
};
