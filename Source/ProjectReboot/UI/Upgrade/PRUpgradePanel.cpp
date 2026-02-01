// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradePanel.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"
#include "ProjectReboot/UI/Upgrade/PRUpgradeListItem.h"
#include "ProjectReboot/UI/Upgrade/PRUpgradeViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"
#include "ProjectReboot/Upgrade/PRUpgradeModuleData.h"

void UPRUpgradePanel::NativeConstruct()
{
	Super::NativeConstruct();

	BindToViewModel();
	RefreshUpgradeList();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UPRUpgradePanel::HandleCloseButtonClicked);
	}
}

void UPRUpgradePanel::NativeDestruct()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
	}

	ClearUpgradeList();
	UnbindFromViewModel();

	Super::NativeDestruct();
}

void UPRUpgradePanel::BindToViewModel()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
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

	BoundViewModel = ViewModel;
	BoundViewModel->OnViewModelUpdated.AddDynamic(this, &ThisClass::HandleViewModelUpdated);
	BoundViewModel->OnVisibilityChanged.AddDynamic(this, &ThisClass::HandleVisibilityChanged);
	// 바인딩 직후 상태 반영
	HandleVisibilityChanged(BoundViewModel->IsVisible());
}

void UPRUpgradePanel::UnbindFromViewModel()
{
	if (BoundViewModel.IsValid())
	{
		BoundViewModel->OnViewModelUpdated.RemoveDynamic(this, &ThisClass::HandleViewModelUpdated);
		BoundViewModel->OnVisibilityChanged.RemoveDynamic(this, &ThisClass::HandleVisibilityChanged);
	}

	BoundViewModel.Reset();
}

void UPRUpgradePanel::HandleCloseButtonClicked()
{
	UPRUIBlueprintLibrary::PopUI(GetOwningPlayer(),this);
}

void UPRUpgradePanel::HandleItemPressed(UPRUpgradeModuleData* InModule)
{
	if (BoundViewModel.IsValid() && IsValid(InModule))
	{
		BoundViewModel->SelectUpgrade(InModule);
	}
}

void UPRUpgradePanel::HandleItemHoldCompleted(UPRUpgradeModuleData* InModule)
{
	if (!BoundViewModel.IsValid() || !IsValid(InModule))
	{
		return;
	}

	if (!BoundViewModel->CanPurchaseModule(InModule))
	{
		return;
	}

	BoundViewModel->SelectUpgrade(InModule);
	BoundViewModel->RequestPurchase();
}

void UPRUpgradePanel::HandleViewModelUpdated()
{
	if (!BoundViewModel.IsValid())
	{
		return;
	}

	const TArray<UPRUpgradeModuleData*>& Modules = BoundViewModel->GetAvailableUpgrades();
	const int32 ModuleCount = Modules.Num();
	const int32 ItemCount = UpgradeListItems.Num();

	if (ModuleCount != ItemCount)
	{
		RefreshUpgradeList();
		return;
	}

	for (int32 Index = 0; Index < ModuleCount; ++Index)
	{
		UPRUpgradeListItem* Item = UpgradeListItems.IsValidIndex(Index) ? UpgradeListItems[Index] : nullptr;
		if (!IsValid(Item) || Item->GetModule() != Modules[Index])
		{
			RefreshUpgradeList();
			return;
		}
	}

	UpdateUpgradeListDisplay();
}

void UPRUpgradePanel::HandleVisibilityChanged(bool bVisible)
{
	// 패널 가시성 적용
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPRUpgradePanel::RefreshUpgradeList()
{
	if (!UpgradeListBox)
	{
		return;
	}

	ClearUpgradeList();

	if (!BoundViewModel.IsValid() || !UpgradeListItemClass)
	{
		return;
	}

	const TArray<UPRUpgradeModuleData*>& Modules = BoundViewModel->GetAvailableUpgrades();
	for (UPRUpgradeModuleData* Module : Modules)
	{
		if (!IsValid(Module))
		{
			continue;
		}

		UPRUpgradeListItem* ItemWidget = CreateWidget<UPRUpgradeListItem>(this, UpgradeListItemClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->InitWidget(Module);
		ItemWidget->OnItemPressed.AddDynamic(this, &ThisClass::HandleItemPressed);
		ItemWidget->OnItemHoldCompleted.AddDynamic(this, &ThisClass::HandleItemHoldCompleted);

		const int32 CurrentLevel = BoundViewModel->GetModuleCurrentLevel(Module);
		const int32 MaxLevel = BoundViewModel->GetModuleMaxLevel(Module);
		const float NextCost = BoundViewModel->GetModuleNextLevelCost(Module);
		ItemWidget->UpdateLevelInfo(CurrentLevel, MaxLevel, NextCost);

		UpgradeListBox->AddChild(ItemWidget);
		UpgradeListItems.Add(ItemWidget);
	}
}

void UPRUpgradePanel::UpdateUpgradeListDisplay()
{
	if (!BoundViewModel.IsValid())
	{
		return;
	}

	for (UPRUpgradeListItem* Item : UpgradeListItems)
	{
		if (IsValid(Item))
		{
			UPRUpgradeModuleData* Module = Item->GetModule();
			if (!IsValid(Module))
			{
				continue;
			}

			Item->UpdateDisplay();
			Item->UpdateLevelInfo(
				BoundViewModel->GetModuleCurrentLevel(Module),
				BoundViewModel->GetModuleMaxLevel(Module),
				BoundViewModel->GetModuleNextLevelCost(Module));
		}
	}
}

void UPRUpgradePanel::ClearUpgradeList()
{
	if (UpgradeListBox)
	{
		UpgradeListBox->ClearChildren();
	}

	UpgradeListItems.Empty();
}
