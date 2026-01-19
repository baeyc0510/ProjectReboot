// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCommonAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjectReboot/Character/PRCharacterBase.h"

void UPRCommonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UPRCommonAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
	{
		UpdateCharacterWalkSpeed();
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// MaxHealth보다 커지지 않도록 클램핑
		if (GetHealth() > NewValue)
		{
			SetHealth(NewValue);
		}
	}
}

void UPRCommonAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
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