// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnemyCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/AI/PRAIConfig.h"


// Sets default values
APREnemyCharacter::APREnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APREnemyCharacter::SetStrafeMode(bool bEnable)
{
	bIsStrafeMode = bEnable;
	GetCharacterMovement()->bOrientRotationToMovement = !bEnable;
	bUseControllerRotationYaw = bEnable;
}