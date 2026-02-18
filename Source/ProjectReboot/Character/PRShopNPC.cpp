// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopNPC.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Camera/PRCameraBlueprintLibrary.h"
#include "ProjectReboot/Shop/PRShopComponent.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"
#include "ProjectReboot/UI/PRUIManagerSubsystem.h"
#include "ProjectReboot/UI/Shop/PRShopPanel.h"

APRShopNPC::APRShopNPC(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ShopComponent = CreateDefaultSubobject<UPRShopComponent>(TEXT("ShopComponent"));

	// 크로스헤어 및 상호작용 UI 감춤
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_HUD_Crosshair, false});
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_HUD_Interaction, false});
}

bool APRShopNPC::CanInteract(APawn* Interactor) const
{
	return IsValid(Interactor);
}

void APRShopNPC::Interact(APawn* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	OpenShopUI(Interactor);
}

FText APRShopNPC::GetInteractionText() const
{
	return InteractionDisplayText;
}

void APRShopNPC::GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const
{
	OutInfo = FPRInteractionInfo();
	OutInfo.DisplayText = GetInteractionText();
	OutInfo.bIsEnabled = CanInteract(Interactor);
}

void APRShopNPC::OpenShopUI(const APawn* Interactor)
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

	UPRUIManagerSubsystem* UIManager = UPRUIBlueprintLibrary::GetUIManager(PlayerController);
	if (!IsValid(UIManager) || !IsValid(ShopPanelClass))
	{
		return;
	}

	UUserWidget* PanelWidget = UIManager->PushUI(ShopPanelClass);
	if (IsValid(PanelWidget))
	{
		// Panel에 ShopComponent 직접 전달
		if (UPRShopPanel* ShopPanel = Cast<UPRShopPanel>(PanelWidget))
		{
			ShopPanel->SetShopComponent(ShopComponent);
		}

		UPRCameraBlueprintLibrary::FocusOnActor(PlayerController, this, FocusParams);
		PanelWidget->OnNativeDestruct.AddUObject(this, &ThisClass::HandleShopPanelDestruct);
	}
}

void APRShopNPC::HandleShopPanelDestruct(UUserWidget* DestructedWidget)
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
