// Fill out your copyright notice in the Description page of Project Settings.

#include "PRModalWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"

void UPRModalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ConfirmButton))
	{
		ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmButtonClicked);
	}
}

void UPRModalWidget::NativeDestruct()
{
	if (IsValid(ConfirmButton))
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleConfirmButtonClicked);
	}

	Super::NativeDestruct();
}

void UPRModalWidget::SetText(const FText& InText)
{
	if (MessageText)
	{
		MessageText->SetText(InText);
	}
}

void UPRModalWidget::HandleConfirmButtonClicked()
{
	UPRUIBlueprintLibrary::PopUI(GetOwningPlayer(), this);
}
