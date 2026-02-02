// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnvQueryContext_AllEnemies.h"
#include "PREQSHelper.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

void UPREnvQueryContext_AllEnemies::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                   FEnvQueryContextData& ContextData) const
{
	// 쿼리를 요청한 주체 가져오기 (AIController든 Pawn이든 안전하게 처리)
	APawn* QuerierPawn = PREQSHelper::GetQuerierPawn(QueryInstance);

	// 월드의 모든 적 캐릭터 가져오기
	TArray<AActor*> AllyActors;
	if (QuerierPawn)
	{
		UGameplayStatics::GetAllActorsOfClass(QuerierPawn->GetWorld(), APREnemyCharacter::StaticClass(), AllyActors);

		// 나 자신(Querier)은 배열에서 제거
		AllyActors.Remove(QuerierPawn);

		// 죽은 액터들 제거
		AllyActors.RemoveAll([](const AActor* Actor)
		{
			if (const IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(Actor))
			{
				return CombatInterface->IsDead();
			}
			return false;
		});
	}

	// EQS 시스템에 데이터 전달
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, AllyActors);
}
