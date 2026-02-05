// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_SpawnRoomRewardActor.generated.h"

class APRRoomController;

/**
 * 방 보상 액터 스폰 Task
 */
USTRUCT()
struct FPRSTT_SpawnRoomRewardActor_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;

	// 보상 액터 클래스
	UPROPERTY(EditAnywhere, Category = Parameter)
	TSubclassOf<AActor> RewardActorClass;

	// 스폰 기준 액터
	UPROPERTY(EditAnywhere, Category = Parameter)
	TObjectPtr<AActor> SpawnPoint = nullptr;

	// RoomController 위치 사용 여부
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bUseRoomControllerTransform = true;

	// 스폰된 보상 액터
	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<AActor> SpawnedActor = nullptr;
};

USTRUCT(meta = (DisplayName = "Spawn Room Reward Actor", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_SpawnRoomRewardActor : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_SpawnRoomRewardActor_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
