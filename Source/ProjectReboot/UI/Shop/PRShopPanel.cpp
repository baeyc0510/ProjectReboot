// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopPanel.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "ProjectReboot/Shop/PRShopComponent.h"

#include "ProjectReboot/Shop/PRShopTypes.h"
#include "PRShopListItem.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"

void UPRShopPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseButtonClicked);
	}
}

void UPRShopPanel::NativeDestruct()
{
	UnbindFromShopComponent();
	ClearShopList();

	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCloseButtonClicked);
	}

	Super::NativeDestruct();
}

void UPRShopPanel::SetShopComponent(UPRShopComponent* InComponent)
{
	UnbindFromShopComponent();
	BoundShopComponent = InComponent;
	BindToShopComponent();

	RefreshShopList();
}

void UPRShopPanel::RefreshShopList()
{
	ClearShopList();

	if (!BoundShopComponent.IsValid() || !IsValid(ShopListItemClass))
	{
		return;
	}

	const TArray<FPRShopItemEntry>& Items = BoundShopComponent->GetShopItems();

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		UPRShopListItem* ListItem = CreateWidget<UPRShopListItem>(this, ShopListItemClass);
		if (!IsValid(ListItem))
		{
			continue;
		}

		ListItem->InitWidget(Items[i].ItemInfo, i);

		ListItem->OnItemPressed.AddDynamic(this, &ThisClass::HandleItemPressed);
		ListItem->OnItemHoldCompleted.AddDynamic(this, &ThisClass::HandleItemHoldCompleted);

		if (IsValid(ShopListBox))
		{
			ShopListBox->AddChild(ListItem);
		}

		ShopListItems.Add(ListItem);
	}

	UpdateShopListDisplay();
	UpdateBalanceDisplay();
}

void UPRShopPanel::UpdateShopListDisplay()
{
	if (!BoundShopComponent.IsValid())
	{
		return;
	}

	const TArray<FPRShopItemEntry>& Items = BoundShopComponent->GetShopItems();

	for (int32 i = 0; i < ShopListItems.Num(); ++i)
	{
		if (!IsValid(ShopListItems[i]) || !Items.IsValidIndex(i))
		{
			continue;
		}

		const bool bCanPurchase = BoundShopComponent->CanPurchaseItem(i);
		ShopListItems[i]->UpdateDisplay(Items[i].bSoldOut, bCanPurchase);
	}
}

void UPRShopPanel::ClearShopList()
{
	if (ShopListBox)
	{
		ShopListBox->ClearChildren();
	}
	
	for (UPRShopListItem* ListItem : ShopListItems)
	{
		if (IsValid(ListItem))
		{
			ListItem->OnItemPressed.RemoveDynamic(this, &ThisClass::HandleItemPressed);
			ListItem->OnItemHoldCompleted.RemoveDynamic(this, &ThisClass::HandleItemHoldCompleted);
			ListItem->RemoveFromParent();
		}
	}
	ShopListItems.Empty();
}

void UPRShopPanel::BindToShopComponent()
{
	if (!BoundShopComponent.IsValid())
	{
		return;
	}

	BoundShopComponent->OnShopInventoryUpdated.AddDynamic(this, &ThisClass::HandleShopInventoryUpdated);
}

void UPRShopPanel::UnbindFromShopComponent()
{
	if (!BoundShopComponent.IsValid())
	{
		return;
	}

	BoundShopComponent->OnShopInventoryUpdated.RemoveDynamic(this, &ThisClass::HandleShopInventoryUpdated);
}

void UPRShopPanel::HandleCloseButtonClicked()
{
	UPRUIBlueprintLibrary::PopUI(GetOwningPlayer(),this);
}

void UPRShopPanel::HandleItemPressed(int32 ItemIndex)
{
	// 눌림 시 선택 표시 등 (필요시 확장)
}

void UPRShopPanel::HandleItemHoldCompleted(int32 ItemIndex)
{
	if (!BoundShopComponent.IsValid())
	{
		return;
	}

	FString FailReason;
	BoundShopComponent->TryPurchaseItem(ItemIndex, FailReason);
}

void UPRShopPanel::UpdateBalanceDisplay()
{
	if (BalanceText && BoundShopComponent.IsValid() && CurrencyTag.IsValid())
	{
		const float Currency = BoundShopComponent->GetCurrency(CurrencyTag);
		BalanceText->SetText(FText::AsNumber(FMath::TruncToInt(Currency)));
	}
}

void UPRShopPanel::HandleShopInventoryUpdated()
{
	UpdateShopListDisplay();
	UpdateBalanceDisplay();
}
