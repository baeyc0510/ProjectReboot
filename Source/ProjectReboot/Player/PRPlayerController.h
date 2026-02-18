// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PRPlayerController.generated.h"

class UPRCrosshairConfig;
class UPRCameraConfig;
class UPRAbilitySystemComponent;
class UPRModalWidget;
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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	/*~ APRPlayerController ~*/
	UPRAbilitySystemComponent* GetPRAbilitySystemComponent() const;

	// 모달 위젯 표시
	UFUNCTION(BlueprintCallable, Category = "UI|Modal")
	UPRModalWidget* ShowModal(const FText& Message);

protected:
	// 크로스헤어 설정
	UPROPERTY(EditDefaultsOnly, Category = "UI|Crosshair")
	TObjectPtr<UPRCrosshairConfig> CrosshairConfig;

	// 모달 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI|Modal")
	TSubclassOf<UPRModalWidget> ModalWidgetClass;

private:
	void InitializeViewModels();
	void DeinitializeViewModels();
};