// Fill out your copyright notice in the Description page of Project Settings.


#include "PRActionListItemWidget.h"

#include "RogueliteActionData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPRActionListItemWidget::InitWidget(URogueliteActionData* InActionData)
{
	if (!TB_ActionName || !TB_Description || !Img_Icon )
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
}