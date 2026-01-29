// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionComponent.h"

#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"

UPRInteractionComponent::UPRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UPRInteractionComponent::InitializeSettings(float InDistance, float InRadius)
{
	InteractionDistance = FMath::Max(0.0f, InDistance);
	InteractionRadius = FMath::Max(0.0f, InRadius);
}

bool UPRInteractionComponent::TryInteract()
{
	APRPlayerCharacter* Player = Cast<APRPlayerCharacter>(GetOwner());
	if (!IsValid(Player))
	{
		return false;
	}

	AActor* InteractableActor = CurrentInteractable.Get();
	if (!IsValid(InteractableActor))
	{
		return false;
	}

	IPRInteractableInterface* InteractableInterface = Cast<IPRInteractableInterface>(InteractableActor);
	if (!InteractableInterface)
	{
		return false;
	}

	if (!InteractableInterface->CanInteract(Player))
	{
		return false;
	}

	InteractableInterface->Interact(Player);
	return true;
}

AActor* UPRInteractionComponent::GetCurrentInteractable() const
{
	return CurrentInteractable.Get();
}

void UPRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateInteractable();
}

void UPRInteractionComponent::UpdateInteractable()
{
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	APawn* Player = Cast<APawn>(OwnerActor);
	if (!IsValid(Player))
	{
		return;
	}

	const FVector Start = OwnerActor->GetActorLocation();
	const FVector End = Start + OwnerActor->GetActorForwardVector() * InteractionDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PRInteractionComponent), false, OwnerActor);
	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(InteractionRadius),
		Params);

	AActor* NewInteractable = nullptr;
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) && HitActor->Implements<UPRInteractableInterface>())
		{
			IPRInteractableInterface* InteractableInterface = Cast<IPRInteractableInterface>(HitActor);
			if (InteractableInterface && InteractableInterface->CanInteract(Player))
			{
				NewInteractable = HitActor;
			}
		}
	}

	if (CurrentInteractable.Get() != NewInteractable)
	{
		CurrentInteractable = NewInteractable;
		OnInteractableChanged.Broadcast(NewInteractable);
	}
}
