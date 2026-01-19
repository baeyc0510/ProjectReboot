// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCommonAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjectReboot/Character/PRCharacterBase.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

void UPRCommonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health 클램핑 (0 ~ MaxHealth)
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
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

void UPRCommonAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Health 변경 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		HandleHealthChanged(Data);
	}
}

void UPRCommonAttributeSet::HandleHealthChanged(const FGameplayEffectModCallbackData& Data)
{
	// Health 클램핑
	const float ClampedHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
	SetHealth(ClampedHealth);

	// 사망 체크
	if (ClampedHealth <= 0.0f)
	{
		HandleDeath(Data);
	}
}

void UPRCommonAttributeSet::HandleDeath(const FGameplayEffectModCallbackData& Data)
{
	AActor* OwnerActor = GetOwningActor();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	// CombatInterface를 통한 사망 처리
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(OwnerActor))
	{
		// 이미 사망 처리 중인 경우 무시
		if (CombatInterface->IsDead())
		{
			return;
		}
		
		const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
		CombatInterface->Die(EffectContext);
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