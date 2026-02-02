// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AIPerceptionTypes.h"
#include "PRAIController.generated.h"

struct FPRAICombatConfig;
class UPRStateTreeAIComponent;
class UStateTreeAIComponent;
class UPRAIConfig;

// 팀 ID 상수
namespace PRTeamId
{
	constexpr uint8 Player = 0;
	constexpr uint8 Enemy = 1;
}

UCLASS()
class PROJECTREBOOT_API APRAIController : public AAIController
{
	GENERATED_BODY()

public:
	APRAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/*~ AAIController Interfaces ~*/
	virtual void OnPossess(APawn* InPawn) override;

	/*~ IGenericTeamAgentInterface ~*/
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(PRTeamId::Enemy); }

	/*~ APRAIController Interfaces ~*/
	virtual void InitializeAI(UPRAIConfig* InAIConfig);

	UFUNCTION(BlueprintPure)
	UPRAIConfig* GetAIConfig() const {return AIConfig;}
	
	UFUNCTION(BlueprintPure)
	AActor* GetCombatTarget() const {return CombatTarget.Get();}
	
	UFUNCTION(BlueprintCallable)
	void SetCombatTarget(AActor* Target) {CombatTarget = Target;}

protected:
	/*~ Perception Event Handlers ~*/
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AI")
	TObjectPtr<UPRStateTreeAIComponent> StateTreeComponent;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRAIConfig> AIConfig;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CombatTarget;
};