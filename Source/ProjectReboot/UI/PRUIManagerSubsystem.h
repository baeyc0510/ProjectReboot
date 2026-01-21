// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PRUIManagerSubsystem.generated.h"

class UUserWidget;

/**
 * UI stack subsystem for local player.
 */
UCLASS()
class PROJECTREBOOT_API UPRUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "WidgetClass"))
	UUserWidget* PushUI(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable)
	void PopUI(UUserWidget* Instance = nullptr);

private:
	void UpdateInputMode();

private:
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> UIStack;
};
