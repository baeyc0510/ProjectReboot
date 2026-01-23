// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "PRAIController.generated.h"

class UPRStateTreeAIComponent;
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
	
	UFUNCTION(BlueprintPure)
	AActor* GetCombatTarget() const {return CombatTarget.Get();}
	
	UFUNCTION(BlueprintCallable)
	void SetCombatTarget(AActor* Target) {CombatTarget = Target;}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AI")
	TObjectPtr<UPRStateTreeAIComponent> StateTreeComponent;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRAIConfig> AIConfig;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CombatTarget;
};