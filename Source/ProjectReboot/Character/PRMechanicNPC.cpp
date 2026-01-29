// Fill out your copyright notice in the Description page of Project Settings.

#include "PRMechanicNPC.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Upgrade/PRUpgradeManagerComponent.h"
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

void APRMechanicNPC::OpenUpgradeUI(const APawn* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!IsValid(GameState))
	{
		return;
	}

	UPRUpgradeManagerComponent* UpgradeManager = GameState->FindComponentByClass<UPRUpgradeManagerComponent>();
	if (!IsValid(UpgradeManager))
	{
		return;
	}

	APlayerController* PlayerController = Interactor->GetController<APlayerController>();
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

	UPRUpgradeViewModel* ViewModel = ViewModelSubsystem->GetOrCreateGlobalViewModel<UPRUpgradeViewModel>();
	if (!IsValid(ViewModel))
	{
		return;
	}

	ViewModel->BindToManager(UpgradeManager);
	ViewModel->SetAvailableUpgrades(AvailableUpgrades);

	UPRUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPRUIManagerSubsystem>();
	if (!IsValid(UIManager) || !IsValid(UpgradePanelClass))
	{
		return;
	}

	UIManager->PushUI(UpgradePanelClass);
}
