// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAIController.h"

#include "PRAIConfig.h"
#include "Perception/AIPerceptionComponent.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "StateTree/PRStateTreeComponent.h"

APRAIController::APRAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	StateTreeComponent = CreateDefaultSubobject<UPRStateTreeComponent>(TEXT("StateTreeComponent"));
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
}

void APRAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (APREnemyCharacter* EnemyCharacter = Cast<APREnemyCharacter>(InPawn))
	{
		check(EnemyCharacter->GetAIConfig());
		InitializeAI(EnemyCharacter->GetAIConfig());
	}
}

void APRAIController::InitializeAI(UPRAIConfig* InAIConfig)
{
	AIConfig = InAIConfig;
	
	check(StateTreeComponent);
	check(AIConfig && AIConfig->StateTree);
	
	if (!StateTreeComponent || !AIConfig || !AIConfig->StateTree)
	{
		return;
	}
	
	// StateTree 에셋 설정
	StateTreeComponent->SetStateTreeAsset(AIConfig->StateTree);
	StateTreeComponent->StartLogic();
}