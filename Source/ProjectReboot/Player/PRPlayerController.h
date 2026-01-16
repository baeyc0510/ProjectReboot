// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PRPlayerController.generated.h"

class UPRCrosshairConfig;
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
	
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/*~ APlayerController Interfaces ~*/
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	
	/*~ APRPlayerController ~*/
	UPRAbilitySystemComponent* GetPRAbilitySystemComponent() const;
	
protected:
	// 크로스헤어 설정 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "UI|Crosshair")
	TObjectPtr<UPRCrosshairConfig> CrosshairConfig;
	
private:
	void InitializeViewModels();
	void DeinitializeViewModels();
};