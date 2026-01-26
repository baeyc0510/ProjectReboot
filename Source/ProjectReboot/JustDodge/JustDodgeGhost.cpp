// Fill out your copyright notice in the Description page of Project Settings.


#include "JustDodgeGhost.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectReboot/PRGameplayTags.h"


// Sets default values
AJustDodgeGhost::AJustDodgeGhost()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCollisionProfileName(TEXT("DodgeGhost"));
	CapsuleComponent->SetGenerateOverlapEvents(true);
}

void AJustDodgeGhost::BeginPlay()
{
	Super::BeginPlay();
	
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOnBeginOverlap);
}

void AJustDodgeGhost::SetOwnerASC(UAbilitySystemComponent* InASC)
{
	OwnerASC = InASC;
}

void AJustDodgeGhost::InitCapsuleSize(UCapsuleComponent* InOriginalCapsule)
{
	if (!InOriginalCapsule)
	{
		return;
	}
	CapsuleComponent->SetCapsuleSize(InOriginalCapsule->GetScaledCapsuleRadius(), InOriginalCapsule->GetScaledCapsuleHalfHeight());
}

void AJustDodgeGhost::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OwnerASC.IsValid())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_HitGhost;
	EventData.Instigator = GetOwner();
	EventData.Target = OtherActor;

	if (bFromSweep)
	{
		EventData.ContextHandle.AddHitResult(SweepResult, true);
	}

	OwnerASC->HandleGameplayEvent(TAG_Event_HitGhost, &EventData);
}
