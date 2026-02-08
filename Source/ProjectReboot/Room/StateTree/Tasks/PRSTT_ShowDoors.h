// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_ShowDoors.generated.h"

class APRRoomController;

/**
 * 방 문 표시 및 활성화 Task
 */
USTRUCT()
struct FPRSTT_ShowDoors_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;
};

USTRUCT(meta = (DisplayName = "Show Doors", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_ShowDoors : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_ShowDoors_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
