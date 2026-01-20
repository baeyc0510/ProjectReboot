// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnemyCharacter.h"

#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"

// Sets default values
APREnemyCharacter::APREnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;
}

void APREnemyCharacter::SetStrafeMode(bool bEnable)
{
	bIsStrafeMode = bEnable;
}

void APREnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this,this);
		AbilitySystem->GiveAbilitySet(EnemyAbilitySet, AbilitySetHandles);
	}
}

void APREnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (AbilitySystem)
	{
		AbilitySetHandles.RemoveFromAbilitySystem();
	}
}
