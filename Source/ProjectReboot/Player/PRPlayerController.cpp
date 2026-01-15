// Fill out your copyright notice in the Description page of Project Settings.


#include "PRPlayerController.h"

#include "AbilitySystemInterface.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"

APRPlayerController::APRPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
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
