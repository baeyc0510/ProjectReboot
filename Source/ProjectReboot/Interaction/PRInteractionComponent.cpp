// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionComponent.h"

#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/ProjectReboot.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "ProjectReboot/UI/Interaction/PRInteractionViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

UPRInteractionComponent::UPRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UPRInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 상호작용 UI ViewModel 초기화
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	InitializeInteractionViewModel(OwnerPawn);
	UpdateInteractionViewModel(OwnerPawn);
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
	
	if (USoundBase* InteractionSound = InteractableInterface->GetInteractionSound())
	{
		UGameplayStatics::PlaySound2D(this, InteractionSound);
	}
	
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
	APlayerController* PlayerController = Cast<APlayerController>(Player->GetController());
	if (!IsValid(PlayerController))
	{
		return;
	}

	// 카메라 정보
	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewForward = ViewRotation.Vector();

	// 화면 크기
	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	// Sphere Overlap으로 범위 내 모든 Interactable 대상 검출,
	// 포커스 상실 여유를 위해 확장된 거리로 검출
	const float OverlapRadius = InteractionDistance + FocusLossMargin;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PRInteractionComponent), false, OwnerActor);
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		EPRCollision::ECC_Interaction,
		FCollisionShape::MakeSphere(OverlapRadius),
		Params);

	// 범위 내에서 화면 중심에 가까운 상호작용 대상 찾기 (동일하면 거리 우선)
	AActor* NewFocusedActor = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	float ClosestScreenDistSq = FLT_MAX;
	const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);
	const float InteractionDistanceSq = InteractionDistance * InteractionDistance;
	const float OverlapRadiusSq = OverlapRadius * OverlapRadius;
	AActor* PrevFocusedActor = CurrentFocusedActor.Get();

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OverlapActor = Result.GetActor();
		if (!IsValid(OverlapActor) || !OverlapActor->Implements<UPRInteractableInterface>())
		{
			continue;
		}

		// 카메라 전방에 있는 액터만 처리
		const FVector ToTarget = OverlapActor->GetActorLocation() - ViewLocation;
		if (FVector::DotProduct(ViewForward, ToTarget.GetSafeNormal()) <= 0.0f)
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(PlayerLocation, OverlapActor->GetActorLocation());

		// 이미 포커스된 대상: 확장 거리까지 허용 / 새 대상: 기본 거리 이내만 허용
		const float AllowedDistanceSq = (OverlapActor == PrevFocusedActor) ? OverlapRadiusSq : InteractionDistanceSq;
		if (DistanceSq > AllowedDistanceSq)
		{
			continue;
		}

		// 화면 내 여부 확인
		FVector2D ScreenPos;
		if (!PlayerController->ProjectWorldLocationToScreen(OverlapActor->GetActorLocation(), ScreenPos, true))
		{
			continue;
		}

		if (ScreenPos.X < 0.0f || ScreenPos.Y < 0.0f ||
			ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
		{
			continue;
		}

		// 화면 중심 거리 계산
		const float ScreenDistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);

		// 화면 중심 우선, 동일하면 거리 우선
		if (ScreenDistSq < ClosestScreenDistSq ||
			(FMath::IsNearlyEqual(ScreenDistSq, ClosestScreenDistSq) && DistanceSq < ClosestDistanceSq))
		{
			ClosestDistanceSq = DistanceSq;
			ClosestScreenDistSq = ScreenDistSq;
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

	// UI 바인딩용 ViewModel 갱신
	UpdateInteractionViewModel(Player);
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

void UPRInteractionComponent::UpdateInteractionViewModel(APawn* Interactor)
{
	if (!InteractionViewModel)
	{
		InitializeInteractionViewModel(Interactor);
	}

	if (!InteractionViewModel)
	{
		return;
	}

	// 포커스 대상 없음 -> UI 숨김 처리
	AActor* FocusedActor = CurrentFocusedActor.Get();
	if (!IsValid(FocusedActor))
	{
		InteractionViewModel->ClearInteractionInfo();
		return;
	}

	IPRInteractableInterface* Interface = Cast<IPRInteractableInterface>(FocusedActor);
	if (!Interface)
	{
		InteractionViewModel->ClearInteractionInfo();
		return;
	}

	// 상호작용 정보 수집 후 UI 노출
	FPRInteractionInfo Info;
	Interface->GetInteractionInfo(Interactor, Info);
	InteractionViewModel->SetInteractionInfo(Info, true);
}

void UPRInteractionComponent::InitializeInteractionViewModel(APawn* Interactor)
{
	if (InteractionViewModel || !IsValid(Interactor))
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Interactor->GetController());
	if (!IsValid(PlayerController))
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UPRViewModelSubsystem* ViewModelSubsystem = LocalPlayer->GetSubsystem<UPRViewModelSubsystem>();
	if (!IsValid(ViewModelSubsystem))
	{
		return;
	}

	InteractionViewModel = Cast<UPRInteractionViewModel>(
		ViewModelSubsystem->GetOrCreateGlobalViewModel(UPRInteractionViewModel::StaticClass()));
}
