// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAIController.h"
#include "PRAIConfig.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"
#include "StateTree/PRStateTreeAIComponent.h"

APRAIController::APRAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	StateTreeComponent = CreateDefaultSubobject<UPRStateTreeAIComponent>(TEXT("StateTreeComponent"));
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &APRAIController::OnTargetPerceptionUpdated);

		// Sight 센스 설정 (적만 감지)
		UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
		if (SightConfig)
		{
			SightConfig->SightRadius = 1500.f;
			SightConfig->LoseSightRadius = 2000.f;
			SightConfig->PeripheralVisionAngleDegrees = 60.f;
			SightConfig->SetMaxAge(5.f);
			SightConfig->DetectionByAffiliation.bDetectEnemies = true;
			SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
			SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
			PerceptionComponent->ConfigureSense(*SightConfig);
		}

		// Hearing 센스 설정 (적만 감지)
		UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
		if (HearingConfig)
		{
			HearingConfig->HearingRange = 2000.f;
			HearingConfig->SetMaxAge(3.f);
			HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
			HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
			HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
			PerceptionComponent->ConfigureSense(*HearingConfig);
		}

		PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	}
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

void APRAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 팀 기반 필터링으로 적(플레이어)만 감지됨
	// 추가 안전 검사로 플레이어 캐스팅
	APRPlayerCharacter* Player = Cast<APRPlayerCharacter>(Actor);
	if (!Player)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// 어떤 sense든 감지되면 타겟으로 설정
		if (!CombatTarget.IsValid())
		{
			SetCombatTarget(Player);
		}
	}
	else
	{
		// 감지가 사라졌을 때, 해당 액터가 현재 타겟이면 해제
		if (CombatTarget == Actor)
		{
			// 다른 sense로 아직 감지 중인지 확인
			if (PerceptionComponent)
			{
				const FActorPerceptionInfo* PerceptionInfo = PerceptionComponent->GetActorInfo(*Actor);
				if (!PerceptionInfo || !PerceptionInfo->HasAnyCurrentStimulus())
				{
					SetCombatTarget(nullptr);
				}
			}
		}
	}
}
