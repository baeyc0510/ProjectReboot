// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PRUIBlueprintLibrary.generated.h"

class UPRUIManagerSubsystem;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category= "UI", meta = (DeterminesOutputType = "WidgetClass"))
	static UUserWidget* PushUI(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass);
	
	UFUNCTION(BlueprintCallable, Category= "UI")
	static void PopUI(APlayerController* OwningPlayer, UUserWidget* WidgetInstance = nullptr);
	
	UFUNCTION(BlueprintCallable, Category= "UI")
	static UPRUIManagerSubsystem* GetUIManager(APlayerController* OwningPlayer);
};
