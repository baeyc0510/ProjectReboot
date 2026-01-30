// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionComponent.h"

#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
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

AActor* UPRInteractionComponent::GetCurrentFocusedActor() const
{
	return CurrentFocusedActor.Get();
}

FText UPRInteractionComponent::GetCurrentInteractionText() const
{
	if (AActor* Actor = CurrentFocusedActor.Get())
	{
		if (IPRInteractableInterface* Interface = Cast<IPRInteractableInterface>(Actor))
		{
			return Interface->GetInteractionText();
		}
	}
	return FText::GetEmpty();
}

bool UPRInteractionComponent::CanCurrentlyInteract() const
{
	return CurrentInteractable.IsValid();
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

	const FVector PlayerLocation = OwnerActor->GetActorLocation();

	// Sphere Overlap으로 범위 내 모든 대상 검출
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PRInteractionComponent), false, OwnerActor);
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(InteractionDistance),
		Params);

	// 범위 내에서 가장 가까운 상호작용 대상 찾기, TODO: 별도 Interaction 채널 사용?
	AActor* NewFocusedActor = nullptr;
	float ClosestDistanceSq = FLT_MAX;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OverlapActor = Result.GetActor();
		if (!IsValid(OverlapActor) || !OverlapActor->Implements<UPRInteractableInterface>())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(PlayerLocation, OverlapActor->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			NewFocusedActor = OverlapActor;
		}
	}

	// 포커스 대상 변경 시 이벤트 호출
	if (CurrentFocusedActor.Get() != NewFocusedActor)
	{
		SetFocusedActor(NewFocusedActor);
	}

	// 상호작용 가능 대상 (CanInteract 체크)
	AActor* NewInteractable = nullptr;
	if (NewFocusedActor)
	{
		IPRInteractableInterface* InteractableInterface = Cast<IPRInteractableInterface>(NewFocusedActor);
		if (InteractableInterface && InteractableInterface->CanInteract(Player))
		{
			NewInteractable = NewFocusedActor;
		}
	}

	if (CurrentInteractable.Get() != NewInteractable)
	{
		CurrentInteractable = NewInteractable;
		OnInteractableChanged.Broadcast(NewInteractable);
	}
}

void UPRInteractionComponent::SetFocusedActor(AActor* NewActor)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	// 이전 대상에게 포커스 상실 알림
	if (AActor* OldActor = CurrentFocusedActor.Get())
	{
		if (IPRInteractableInterface* Interface = Cast<IPRInteractableInterface>(OldActor))
		{
			Interface->OnLoseInteractFocus(OwnerPawn);
		}
	}

	CurrentFocusedActor = NewActor;

	// 새 대상에게 포커스 획득 알림
	if (NewActor)
	{
		if (IPRInteractableInterface* Interface = Cast<IPRInteractableInterface>(NewActor))
		{
			Interface->OnGainInteractFocus(OwnerPawn);
		}
	}
}
