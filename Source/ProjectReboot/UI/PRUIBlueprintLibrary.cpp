// Fill out your copyright notice in the Description page of Project Settings.


#include "PRUIBlueprintLibrary.h"

#include "PRUIManagerSubsystem.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

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

UPRViewModelSubsystem* UPRUIBlueprintLibrary::GetViewModelSubsystem(APlayerController* OwningPlayer)
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

	return LocalPlayer->GetSubsystem<UPRViewModelSubsystem>();
}

UPRViewModelBase* UPRUIBlueprintLibrary::RegisterGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->RegisterGlobalViewModel(ViewModelClass);
	}

	return nullptr;
}

UPRViewModelBase* UPRUIBlueprintLibrary::GetGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->GetGlobalViewModel(ViewModelClass);
	}

	return nullptr;
}

UPRViewModelBase* UPRUIBlueprintLibrary::GetOrCreateGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->GetOrCreateGlobalViewModel(ViewModelClass);
	}

	return nullptr;
}

bool UPRUIBlueprintLibrary::UnregisterGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->UnregisterGlobalViewModel(ViewModelClass);
	}

	return false;
}

UPRViewModelBase* UPRUIBlueprintLibrary::GetOrCreateActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->GetOrCreateActorViewModel(TargetActor, ViewModelClass);
	}

	return nullptr;
}

UPRViewModelBase* UPRUIBlueprintLibrary::FindActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->FindActorViewModel(TargetActor, ViewModelClass);
	}

	return nullptr;
}

bool UPRUIBlueprintLibrary::RemoveActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		return ViewModelSubsystem->RemoveActorViewModel(TargetActor, ViewModelClass);
	}

	return false;
}

void UPRUIBlueprintLibrary::RemoveAllViewModelsForActor(APlayerController* OwningPlayer, AActor* TargetActor)
{
	if (UPRViewModelSubsystem* ViewModelSubsystem = GetViewModelSubsystem(OwningPlayer))
	{
		ViewModelSubsystem->RemoveAllViewModelsForActor(TargetActor);
	}
}
