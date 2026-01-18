// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCharacterBase.h"

#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"


// Sets default values
APRCharacterBase::APRCharacterBase()
{
	AbilitySystem = CreateDefaultSubobject<UPRAbilitySystemComponent>(TEXT("AbilitySystem"));
	CommonAttributeSet = CreateDefaultSubobject<UPRCommonAttributeSet>(TEXT("CommonAttributeSet"));
}

UAbilitySystemComponent* APRCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}
