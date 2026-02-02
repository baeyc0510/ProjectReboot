// Fill out your copyright notice in the Description page of Project Settings.

#include "PREnvQueryTest_CombatDistance.h"
#include "PREQSHelper.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "ProjectReboot/AI/PRAIConfig.h"

UPREnvQueryTest_CombatDistance::UPREnvQueryTest_CombatDistance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(true);
}

void UPREnvQueryTest_CombatDistance::RunTest(FEnvQueryInstance& QueryInstance) const
{
	// QueryOwner가 AIController든 Pawn이든 안전하게 처리
	APRAIController* AIController = PREQSHelper::GetQuerierPRAIController(QueryInstance);
	if (!AIController)
	{
		return;
	}

	UPRAIConfig* AIConfig = AIController->GetAIConfig();
	if (!AIConfig)
	{
		return;
	}

	const FPRAICombatConfig& CombatConfig = AIConfig->CombatConfig;
	const float IdealRange = CombatConfig.IdealRange;

	// 타겟 위치 가져오기
	AActor* CombatTarget = AIController->GetCombatTarget();
	if (!CombatTarget)
	{
		return;
	}

	const FVector TargetLocation = CombatTarget->GetActorLocation();

	const float MinRange = CombatConfig.MinRange;
	const float MaxRange = CombatConfig.MaxCombatRange;

	// 각 아이템에 대해 IdealRange로부터의 편차를 계산
	// 엔진의 ScoringEquation (Inverse Linear)이 적용되어 IdealRange에 가까울수록 높은 점수
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		const float Distance = FVector::Dist(ItemLocation, TargetLocation);

		// |Distance - IdealRange|: IdealRange에서 0, 멀어질수록 증가
		const float DeviationFromIdeal = FMath::Abs(Distance - IdealRange);

		// FilterMin/FilterMax는 Filter 모드에서만 사용됨
		It.SetScore(TestPurpose, FilterType, DeviationFromIdeal, MinRange, MaxRange);
	}
}

FText UPREnvQueryTest_CombatDistance::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("PR Combat Distance"));
}

FText UPREnvQueryTest_CombatDistance::GetDescriptionDetails() const
{
	return FText::FromString(TEXT("CombatConfig의 IdealRange 기준 거리 편차 (Inverse Linear 권장)"));
}
