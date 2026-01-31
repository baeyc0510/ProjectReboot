// Fill out your copyright notice in the Description page of Project Settings.

#include "PRCameraBlueprintLibrary.h"

#include "PRActorFocusSubsystem.h"

/*~ 액터 포커스 ~*/

UPRActorFocusSubsystem* UPRCameraBlueprintLibrary::GetActorFocusSubsystem(APlayerController* OwningPlayer)
{
	if (!IsValid(OwningPlayer))
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UPRActorFocusSubsystem>();
}

bool UPRCameraBlueprintLibrary::FocusOnActor(APlayerController* OwningPlayer, AActor* TargetActor, const FActorFocusParams& Params)
{
	if (UPRActorFocusSubsystem* FocusSubsystem = GetActorFocusSubsystem(OwningPlayer))
	{
		return FocusSubsystem->FocusOnActor(TargetActor, Params);
	}

	return false;
}

void UPRCameraBlueprintLibrary::RestoreFocus(APlayerController* OwningPlayer)
{
	if (UPRActorFocusSubsystem* FocusSubsystem = GetActorFocusSubsystem(OwningPlayer))
	{
		FocusSubsystem->RestoreFocus();
	}
}

bool UPRCameraBlueprintLibrary::IsFocusing(APlayerController* OwningPlayer)
{
	if (UPRActorFocusSubsystem* FocusSubsystem = GetActorFocusSubsystem(OwningPlayer))
	{
		return FocusSubsystem->IsFocusing();
	}

	return false;
}