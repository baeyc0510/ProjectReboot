// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

UUserWidget* UPRUIManagerSubsystem::PushUI(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(World, WidgetClass);
	if (!Widget)
	{
		return nullptr;
	}

	Widget->AddToViewport();
	UIStack.Add(Widget);
	UpdateInputMode();
	return Widget;
}

void UPRUIManagerSubsystem::PopUI(UUserWidget* Instance)
{
	if (UIStack.IsEmpty())
	{
		return;
	}

	UUserWidget* Target = Instance;
	if (!Target)
	{
		Target = UIStack.Last();
	}

	if (Target)
	{
		UIStack.Remove(Target);
		Target->RemoveFromParent();
	}

	UpdateInputMode();
}

void UPRUIManagerSubsystem::UpdateInputMode()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(World);
	if (!PlayerController)
	{
		return;
	}

	if (UIStack.Num() > 0)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(UIStack.Last()->TakeWidget());
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
		PlayerController->FlushPressedKeys();
	}
	else
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = false;
	}
}
