// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCommonAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPRCommonAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute() || Attribute == GetMoveSpeedMultiplierAttribute())
	{
		UpdateCharacterWalkSpeed();
	}
}

void UPRCommonAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UPRCommonAttributeSet::UpdateCharacterWalkSpeed()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
	{
		if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
		{
			float MoveSpeedCoeff = GetMoveSpeedMultiplier();
			float BaseMoveSpeed = GetMoveSpeed();
			CMC->MaxWalkSpeed =  MoveSpeedCoeff * BaseMoveSpeed;
		}
	}
}
