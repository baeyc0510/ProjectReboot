// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "PRSTT_SpawnWave.generated.h"

class APRRoomController;

/**
 * 스폰 큐 항목
 */
USTRUCT()
struct FPRSpawnQueueEntry
{
	GENERATED_BODY()

	// 스폰할 적 클래스
	UPROPERTY()
	TSubclassOf<APREnemyCharacter> EnemyClass;

	// 남은 스폰 수
	UPROPERTY()
	int32 RemainingCount = 0;
};

/**
 * 웨이브 스폰 Task Instance Data
 */
USTRUCT()
struct FPRSTT_SpawnWave_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;

	// 프레임당 스폰 수
	UPROPERTY(EditAnywhere, Category = Parameter)
	int32 SpawnPerTick = 2;

	// 전체 스폰 예정 수 (Output)
	UPROPERTY(EditAnywhere, Category = Output)
	int32 TotalToSpawn = 0;
	
	// 스폰 큐 (내부)
	TArray<FPRSpawnQueueEntry> SpawnQueue;

	// 스폰 포인트 인덱스 (내부)
	int32 SpawnPointIndex = 0;

	// 스폰된 적 수
	int32 SpawnedCount = 0;

	// 스폰 진행 중 플래그 (Sustained 재진입 방어)
	bool bSpawning = false;
};

/**
 * 웨이브 스폰 StateTree Task
 * Tick 기반으로 프레임당 SpawnPerTick개씩 순차 스폰
 */
USTRUCT(meta = (DisplayName = "Spawn Wave", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_SpawnWave : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_SpawnWave_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 상태 진입 시 스폰 큐 준비
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	// 매 틱 순차 스폰
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
	
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	// 큐에서 다음 적 1체 스폰
	bool SpawnNextEnemy(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};
