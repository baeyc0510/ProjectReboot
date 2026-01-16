// Fill out your copyright notice in the Description page of Project Settings.


#include "PRPlayerController.h"

#include "AbilitySystemInterface.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/UI/Crosshair/PRCrosshairViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

APRPlayerController::APRPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void APRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitializeViewModels();
}

void APRPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeinitializeViewModels();
	Super::EndPlay(EndPlayReason);
}

void APRPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UPRAbilitySystemComponent* PRASC = GetPRAbilitySystemComponent())
	{
		PRASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

UPRAbilitySystemComponent* APRPlayerController::GetPRAbilitySystemComponent() const
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPawn()))
	{
		return Cast<UPRAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}
	return nullptr;
}

void APRPlayerController::InitializeViewModels()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
	if (!VMS)
	{
		return;
	}

	// Crosshair ViewModel 초기화
	if (CrosshairConfig)
	{
		UPRCrosshairViewModel* CrosshairVM = VMS->GetOrCreateGlobalViewModel<UPRCrosshairViewModel>();
		CrosshairVM->SetConfig(CrosshairConfig);
	}
}

void APRPlayerController::DeinitializeViewModels()
{
}
