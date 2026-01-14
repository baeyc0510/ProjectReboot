// Fill out your copyright notice in the Description page of Project Settings.

#include "PRActionListItemWidget.h"

#include "RogueliteActionData.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPRActionListItemWidget::InitWidget(URogueliteActionData* InActionData)
{
	ActionData = InActionData;

	if (!TB_ActionName || !TB_Description || !Img_Icon)
	{
		return;
	}

	if (!InActionData)
	{
		TB_ActionName->SetText(FText());
		TB_Description->SetText(FText());
		Img_Icon->SetBrushFromTexture(nullptr);
		return;
	}

	FText DisplayNameText = InActionData->GetFormattedTextWithValues(InActionData->DisplayName);
	FText DescriptionText = InActionData->GetFormattedTextWithValues(InActionData->Description);

	TB_ActionName->SetText(DisplayNameText);
	TB_Description->SetText(DescriptionText);
	Img_Icon->SetBrushFromTexture(InActionData->Icon);

	// 초기 Border 색상 설정
	SetSelected(false);
}

void UPRActionListItemWidget::SetSelected(bool bInSelected)
{
	bIsSelected = bInSelected;

	if (Border_Background)
	{
		Border_Background->SetBrushColor(bIsSelected ? SelectedBorderColor : NormalBorderColor);
	}
}

FReply UPRActionListItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnClicked.Broadcast(this);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}