// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_SpawnWave.generated.h"

class APRRoomController;

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

	// 스폰된 적 수 (Output)
	UPROPERTY(EditAnywhere, Category = Output)
	int32 SpawnedCount = 0;
};

/**
 * 웨이브 스폰 StateTree Task
 * 지정된 웨이브의 적을 스폰 (즉시 완료)
 */
USTRUCT(meta = (DisplayName = "Spawn Wave", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_SpawnWave : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_SpawnWave_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

protected:
	// 웨이브의 적 스폰
	int32 SpawnWaveEnemies(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};
