// Fill out your copyright notice in the Description page of Project Settings.

#include "PRBillboardInfoWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPRBillboardInfoWidget::SetDisplayText(const FText& InText)
{
	if (IsValid(DisplayText))
	{
		DisplayText->SetText(InText);
	}
}

void UPRBillboardInfoWidget::SetIcon(UTexture2D* InIcon)
{
	if (IsValid(IconImage))
	{
		IconImage->SetBrushFromTexture(InIcon);
		IconImage->SetVisibility(InIcon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPRBillboardInfoWidget::SetContent(const FText& InText, UTexture2D* InIcon)
{
	SetDisplayText(InText);
	SetIcon(InIcon);
}
