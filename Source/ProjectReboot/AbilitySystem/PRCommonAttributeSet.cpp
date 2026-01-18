// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCommonAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/Character/PRCharacterBase.h"

void UPRCommonAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
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
	if (APRCharacterBase* Character = Cast<APRCharacterBase>(GetOwningActor()))
	{
		if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
		{
			CMC->MaxWalkSpeed = Character->GetBaseMovementSpeed() * GetMoveSpeed();
		}
	}
}
