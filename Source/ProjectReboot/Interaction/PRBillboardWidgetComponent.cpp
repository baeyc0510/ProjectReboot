// Fill out your copyright notice in the Description page of Project Settings.

#include "PRBillboardWidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UPRBillboardWidgetComponent::UPRBillboardWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(true);
}

void UPRBillboardWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableBillboard || !IsVisible())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (!CameraManager)
	{
		return;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
		GetComponentLocation(), CameraLocation);

	if (!bRotatePitch)
	{
		LookAtRotation.Pitch = 0.0f;
	}

	SetWorldRotation(LookAtRotation);
}
