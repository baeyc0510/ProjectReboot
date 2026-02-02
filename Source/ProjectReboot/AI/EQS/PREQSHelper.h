// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "ProjectReboot/AI/PRAIController.h"

/**
 * EQS 클래스들을 위한 헬퍼 함수 모음
 * QueryOwner가 AIController든 Pawn이든 안전하게 처리
 */
namespace PREQSHelper
{
	// QueryOwner에서 Pawn 가져오기 (Owner가 Controller든 Pawn이든 동작)
	inline APawn* GetQuerierPawn(const FEnvQueryInstance& QueryInstance)
	{
		UObject* Owner = QueryInstance.Owner.Get();
		if (!Owner)
		{
			return nullptr;
		}

		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Controller->GetPawn();
		}

		return Cast<APawn>(Owner);
	}

	// QueryOwner에서 AIController 가져오기 (Owner가 Controller든 Pawn이든 동작)
	inline AAIController* GetQuerierController(const FEnvQueryInstance& QueryInstance)
	{
		UObject* Owner = QueryInstance.Owner.Get();
		if (!Owner)
		{
			return nullptr;
		}

		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Controller;
		}

		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAIController>(Pawn->GetController());
		}

		return nullptr;
	}

	// QueryOwner에서 PRAIController 가져오기 (프로젝트 전용)
	inline APRAIController* GetQuerierPRAIController(const FEnvQueryInstance& QueryInstance)
	{
		return Cast<APRAIController>(GetQuerierController(QueryInstance));
	}
}
