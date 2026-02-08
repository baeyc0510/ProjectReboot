// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_SetRoomDoorsInteractable.generated.h"

class APRRoomController;

/**
 * 방 문 상호작용 상태 설정 Task
 */
USTRUCT()
struct FPRSTT_SetRoomDoorsInteractable_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;
	
	// 상호작용 가능 여부
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bInteractable = true;
};

USTRUCT(meta = (DisplayName = "Set Room Doors Interactable", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_SetRoomDoorsInteractable : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_SetRoomDoorsInteractable_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
