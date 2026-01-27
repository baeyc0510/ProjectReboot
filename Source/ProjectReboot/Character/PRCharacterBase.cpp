// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCharacterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/Animation/PRMontageSet.h"


// Sets default values
APRCharacterBase::APRCharacterBase()
{
	AbilitySystem = CreateDefaultSubobject<UPRAbilitySystemComponent>(TEXT("AbilitySystem"));
	CommonAttributeSet = CreateDefaultSubobject<UPRCommonAttributeSet>(TEXT("CommonAttributeSet"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("CombatCharacter"));
	
	CombatCapsuleComponent = GetCapsuleComponent();
}

UAbilitySystemComponent* APRCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

UCapsuleComponent* APRCharacterBase::GetCombatCapsuleComponent() const
{
	if (CombatCapsuleComponent)
	{
		return CombatCapsuleComponent;
	}
	return GetCapsuleComponent();
}

bool APRCharacterBase::IsDead() const
{
	if (AbilitySystem)
	{
		return AbilitySystem->HasMatchingGameplayTag(TAG_State_Dead);
	}
	return false;
}

void APRCharacterBase::Die(const FGameplayEffectContextHandle& EffectContext)
{
	if (!AbilitySystem)
	{
		return;
	}
	
	if (IsDead())
	{
		return;
	}
	
	HandleCollisionAndMovementOnDeath();
	
	// GameplayEvent를 통한 GA_Die 트리거 방식.
	// GA_Die를 부여해야하며, Trigger로 "Event.Death"가 설정되어 있어야 함 
	FGameplayEventData EventData;
	{
		EventData.EventTag = TAG_Event_Death;
		EventData.ContextHandle = EffectContext;
		if (EffectContext.IsValid())
		{
			EventData.Instigator = EffectContext.GetInstigator();
			EventData.OptionalObject = EffectContext.GetEffectCauser();
		}
		EventData.Target = this;
	
		// Tags
		if (IsValid(EventData.Instigator))
		{
			if (UAbilitySystemComponent* InstigatorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EventData.Instigator))
			{
				InstigatorASC->GetOwnedGameplayTags(EventData.InstigatorTags);
			}
		}
		AbilitySystem->GetOwnedGameplayTags(EventData.TargetTags);
	}
	
	// 이벤트 전송
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor( this,TAG_Event_Death,EventData);
	
	
}

void APRCharacterBase::FinishDie()
{
}

void APRCharacterBase::OnHit(const FHitResult& HitResult)
{
	if (!AbilitySystem)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_Hit;
	EventData.Target = this;
	EventData.ContextHandle.AddHitResult(HitResult, true);

	AbilitySystem->HandleGameplayEvent(TAG_Event_Hit, &EventData);
}

UAnimMontage* APRCharacterBase::FindMontageByGameplayTag(const FGameplayTag& MontageTag) const
{
	if (MontageSet)
	{
		return MontageSet->FindMontageByTag(MontageTag);
	}
	return nullptr;
}

float APRCharacterBase::GetMaxMoveSpeed() const
{
	if (CommonAttributeSet)
	{
		return CommonAttributeSet->GetMoveSpeed() * BaseMoveSpeed;
	}
	return BaseMoveSpeed;
}

void APRCharacterBase::AddDefaultAbilitySystemTags() const
{
	if (AbilitySystem)
	{
		AbilitySystem->AddLooseGameplayTags(DefaultAbilitySystemTags);
	}
}

void APRCharacterBase::HandleCollisionAndMovementOnDeath()
{
	// 콜리전 비활성화
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 이동 비활성화
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
		MovementComp->SetComponentTickEnabled(false);
	}
}
