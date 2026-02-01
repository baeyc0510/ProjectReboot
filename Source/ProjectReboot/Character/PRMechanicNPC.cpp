// Fill out your copyright notice in the Description page of Project Settings.

#include "PRMechanicNPC.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Camera/PRCameraBlueprintLibrary.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"
#include "ProjectReboot/UI/Upgrade/PRUpgradeViewModel.h"
#include "ProjectReboot/UI/Upgrade/PRUpgradePanel.h"
#include "ProjectReboot/UI/PRUIManagerSubsystem.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

APRMechanicNPC::APRMechanicNPC(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 크로스헤어 및 상호작용 UI 감춤
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_Crosshair,false});
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_Interaction,false});
}

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
	
	InteractorPawn = Interactor;
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

	UUserWidget* PanelWidget = UIManager->PushUI(UpgradePanelClass);
	if (IsValid(PanelWidget))
	{
		UPRCameraBlueprintLibrary::FocusOnActor(PlayerController, this, FocusParams);
		PanelWidget->OnNativeDestruct.AddUObject(this, &ThisClass::HandleUpgradePanelDestruct);
	}
}

void APRMechanicNPC::HandleUpgradePanelDestruct(UUserWidget* DestructedWidget)
{
	if (!InteractorPawn.IsValid())
	{
		return;
	}
	
	APlayerController* PlayerController = InteractorPawn->GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}

	UPRCameraBlueprintLibrary::RestoreFocus(PlayerController);
	InteractorPawn.Reset();
}
