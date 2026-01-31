// Fill out your copyright notice in the Description page of Project Settings.

#include "PRMechanicNPC.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"
#include "ProjectReboot/UI/Upgrade/PRUpgradeViewModel.h"
#include "ProjectReboot/UI/PRUIManagerSubsystem.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

bool APRMechanicNPC::CanInteract(APawn* Interactor) const
{
	return IsValid(Interactor);
}

void APRMechanicNPC::Interact(APawn* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	OpenUpgradeUI(Interactor);
}

FText APRMechanicNPC::GetInteractionText() const
{
	return InteractionDisplayText;
}

void APRMechanicNPC::GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const
{
	OutInfo = FPRInteractionInfo();
	OutInfo.DisplayText = GetInteractionText();
	OutInfo.bIsEnabled = CanInteract(Interactor);
}

void APRMechanicNPC::OpenUpgradeUI(const APawn* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	APlayerController* PlayerController = Interactor->GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	
	UPRViewModelSubsystem* ViewModelSubsystem = UPRUIBlueprintLibrary::GetViewModelSubsystem(PlayerController);
	if (!IsValid(ViewModelSubsystem))
	{
		return;
	}

	UPRUpgradeViewModel* ViewModel = ViewModelSubsystem->GetOrCreateGlobalViewModel<UPRUpgradeViewModel>();
	if (!IsValid(ViewModel))
	{
		return;
	}
	
	// ViewModel 갱신
	ViewModel->SetAvailableUpgrades(AvailableUpgrades);

	UPRUIManagerSubsystem* UIManager = UPRUIBlueprintLibrary::GetUIManager(PlayerController); 
	if (!IsValid(UIManager) || !IsValid(UpgradePanelClass))
	{
		return;
	}

	UIManager->PushUI(UpgradePanelClass);
}
