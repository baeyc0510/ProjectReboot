// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_WaveSpawn.generated.h"

class APRRoomController;

/**
 * 웨이브 스폰 Task Instance Data
 */
USTRUCT()
struct FPRSTT_WaveSpawn_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;

	// 웨이브 인덱스
	UPROPERTY(EditAnywhere, Category = Parameter)
	int32 WaveIndex = 0;

	// 스폰된 적 수 (런타임)
	int32 SpawnedCount = 0;

	// 델리게이트 핸들
	FDelegateHandle KillCountHandle;
};

/**
 * 웨이브 스폰 StateTree Task
 * 지정된 웨이브의 적을 스폰하고 클리어 감지
 */
USTRUCT(meta = (DisplayName = "Wave Spawn", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_WaveSpawn : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_WaveSpawn_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

protected:
	// 웨이브의 적 스폰
	int32 SpawnWaveEnemies(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};
