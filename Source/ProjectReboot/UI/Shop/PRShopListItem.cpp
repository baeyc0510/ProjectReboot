// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopListItem.h"

#include "RogueliteActionData.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"


void UPRShopListItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (PurchaseButton)
	{
		PurchaseButton->OnPressed.AddDynamic(this, &ThisClass::HandleButtonPressed);
		PurchaseButton->OnReleased.AddDynamic(this, &ThisClass::HandleButtonReleased);
		PurchaseButton->OnHovered.AddDynamic(this, &ThisClass::HandleButtonHovered);
		PurchaseButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleButtonUnhovered);
	}

	// 초기 상태
	UpdateProgressBar(0.0f);

	if (SoldOutOverlay)
	{
		SoldOutOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPRShopListItem::NativeDestruct()
{
	// 홀드 중단 및 바인딩 해제
	StopHold(true);

	if (PurchaseButton)
	{
		PurchaseButton->OnPressed.RemoveAll(this);
		PurchaseButton->OnReleased.RemoveAll(this);
		PurchaseButton->OnHovered.RemoveAll(this);
		PurchaseButton->OnUnhovered.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPRShopListItem::InitWidget(const FPRShopItemInfo& InItemInfo, int32 InItemIndex)
{
	CachedItemInfo = InItemInfo;
	ItemIndex = InItemIndex;
	RefreshDisplay();
}

void UPRShopListItem::UpdateDisplay(bool bInSoldOut, bool bCanPurchase)
{
	bIsSoldOut = bInSoldOut;

	if (SoldOutOverlay)
	{
		SoldOutOverlay->SetVisibility(bIsSoldOut ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (PurchaseButton)
	{
		PurchaseButton->SetIsEnabled(!bIsSoldOut && bCanPurchase);
	}
}

void UPRShopListItem::HandleButtonPressed()
{
	if (bIsSoldOut)
	{
		return;
	}

	OnItemPressed.Broadcast(ItemIndex);
	StartHold();
}

void UPRShopListItem::HandleButtonReleased()
{
	StopHold(true);
}

void UPRShopListItem::HandleButtonHovered()
{
	if (HoverAnimation)
	{
		PlayAnimationForward(HoverAnimation);
	}

	bIsHovered = true;
}

void UPRShopListItem::HandleButtonUnhovered()
{
	if (HoverAnimation)
	{
		PlayAnimationReverse(HoverAnimation);
	}

	bIsHovered = false;
	StopHold(true);
}

void UPRShopListItem::StartHold()
{
	if (bIsHolding)
	{
		return;
	}

	if (ItemIndex == INDEX_NONE)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bIsHolding = true;
	HoldElapsed = 0.0f;
	UpdateProgressBar(0.0f);

	World->GetTimerManager().SetTimer(HoldTimerHandle, this, &ThisClass::HandleHoldTick, HoldTickInterval, true);
}

void UPRShopListItem::StopHold(bool bResetProgress)
{
	if (!bIsHolding)
	{
		if (bResetProgress)
		{
			UpdateProgressBar(0.0f);
		}
		return;
	}

	bIsHolding = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldTimerHandle);
	}

	if (bResetProgress)
	{
		UpdateProgressBar(0.0f);
	}
}

void UPRShopListItem::HandleHoldTick()
{
	if (!bIsHolding)
	{
		return;
	}

	HoldElapsed += HoldTickInterval;
	const float Progress = HoldDuration > 0.0f ? HoldElapsed / HoldDuration : 1.0f;
	UpdateProgressBar(FMath::Clamp(Progress, 0.0f, 1.0f));

	if (HoldElapsed >= HoldDuration)
	{
		HandleHoldCompleted();
	}
}

void UPRShopListItem::HandleHoldCompleted()
{
	StopHold(false);

	OnItemHoldCompleted.Broadcast(ItemIndex);

	UpdateProgressBar(0.0f);
}

void UPRShopListItem::UpdateProgressBar(float Progress)
{
	if (PB_HoldProgress)
	{
		PB_HoldProgress->SetPercent(Progress);
	}
}

void UPRShopListItem::RefreshDisplay()
{
	if (!IsValid(CachedItemInfo.ActionData))
	{
		if (TB_Name)
		{
			TB_Name->SetText(FText::GetEmpty());
		}

		if (TB_Description)
		{
			TB_Description->SetText(FText::GetEmpty());
		}

		if (Img_Icon)
		{
			Img_Icon->SetBrushFromTexture(nullptr);
		}

		if (TB_Price)
		{
			TB_Price->SetText(FText::GetEmpty());
		}

		return;
	}

	if (TB_Name)
	{
		TB_Name->SetText(CachedItemInfo.GetDisplayName());
	}

	if (TB_Description)
	{
		TB_Description->SetText(CachedItemInfo.GetDescription());
	}

	if (Img_Icon)
	{
		Img_Icon->SetBrushFromTexture(CachedItemInfo.GetIcon());
	}

	if (TB_Price)
	{
		TB_Price->SetText(FText::Format(NSLOCTEXT("Shop", "Price", "{0}"),
			FText::AsNumber(static_cast<int32>(CachedItemInfo.Price))));
	}
}
