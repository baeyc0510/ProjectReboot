// Fill out your copyright notice in the Description page of Project Settings.


#include "PRUIBlueprintLibrary.h"

#include "PRUIManagerSubsystem.h"

UUserWidget* UPRUIBlueprintLibrary::PushUI(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (UPRUIManagerSubsystem* UIManager = GetUIManager(OwningPlayer))
	{
		return UIManager->PushUI(WidgetClass);
	}
	
	return nullptr;
}

void UPRUIBlueprintLibrary::PopUI(APlayerController* OwningPlayer, UUserWidget* WidgetInstance)
{
	if (UPRUIManagerSubsystem* UIManager = GetUIManager(OwningPlayer))
	{
		UIManager->PopUI(WidgetInstance);
	}
}

UPRUIManagerSubsystem* UPRUIBlueprintLibrary::GetUIManager(APlayerController* OwningPlayer)
{
	if (!IsValid(OwningPlayer))
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return nullptr;
	}
	
	return LocalPlayer->GetSubsystem<UPRUIManagerSubsystem>();
}
