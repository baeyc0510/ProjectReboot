// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "PRAIController.generated.h"

class UPRStateTreeComponent;
class UStateTreeAIComponent;
class UPRAIConfig;

UCLASS()
class PROJECTREBOOT_API APRAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APRAIController();
	
	/*~ AAIController Interfaces ~*/
	virtual void OnPossess(APawn* InPawn) override;

	/*~ APRAIController Interfaces ~*/
	virtual void InitializeAI(UPRAIConfig* InAIConfig);

	UPRAIConfig* GetAIConfig() const {return AIConfig;}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AI")
	TObjectPtr<UPRStateTreeComponent> StateTreeComponent;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRAIConfig> AIConfig;
};