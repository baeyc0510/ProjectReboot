// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PRPlayerController.generated.h"

class UPRAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API APRPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APRPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	UPRAbilitySystemComponent* GetPRAbilitySystemComponent() const;
};
