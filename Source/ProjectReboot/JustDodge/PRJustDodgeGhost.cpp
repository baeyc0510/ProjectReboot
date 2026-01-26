// Fill out your copyright notice in the Description page of Project Settings.


#include "PRJustDodgeGhost.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectReboot/PRGameplayTags.h"


// Sets default values
APRJustDodgeGhost::APRJustDodgeGhost()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCollisionProfileName(TEXT("DodgeGhost"));
}

void APRJustDodgeGhost::BeginPlay()
{
	Super::BeginPlay();
}

void APRJustDodgeGhost::SetOwnerASC(UAbilitySystemComponent* InASC)
{
	OwnerASC = InASC;
}

void APRJustDodgeGhost::InitCapsuleSize(UCapsuleComponent* InOriginalCapsule)
{
	if (!InOriginalCapsule)
	{
		return;
	}
	CapsuleComponent->SetCapsuleSize(InOriginalCapsule->GetScaledCapsuleRadius(), InOriginalCapsule->GetScaledCapsuleHalfHeight());
}

bool APRJustDodgeGhost::IsDead() const
{
	return false;
}

void APRJustDodgeGhost::Die(const FGameplayEffectContextHandle& EffectContext)
{
}

void APRJustDodgeGhost::FinishDie()
{
}

void APRJustDodgeGhost::OnHit(const FHitResult& HitResult)
{
	if (!OwnerASC.IsValid())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_HitGhost;
	EventData.Instigator = GetOwner();
	EventData.Target = this;
	EventData.ContextHandle.AddHitResult(HitResult, true);

	OwnerASC->HandleGameplayEvent(TAG_Event_HitGhost, &EventData);
}