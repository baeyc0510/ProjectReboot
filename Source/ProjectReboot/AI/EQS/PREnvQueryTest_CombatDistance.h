// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "PREnvQueryTest_CombatDistance.generated.h"

/**
 * FPRAICombatConfig 기반 거리 스코어링 테스트
 * IdealRange로부터의 편차를 반환 (에디터에서 Inverse Linear 권장)
 */
UCLASS()
class PROJECTREBOOT_API UPREnvQueryTest_CombatDistance : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UPREnvQueryTest_CombatDistance(const FObjectInitializer& ObjectInitializer);

	/*~ UEnvQueryTest Interface ~*/
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
