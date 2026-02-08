// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "PRSTT_SendRoomEvent.generated.h"

/**
 * 방 이벤트 전송 Task Instance Data
 */
USTRUCT()
struct FPRSTT_SendRoomEvent_InstanceData
{
	GENERATED_BODY()

	// 전송할 이벤트 태그 (Parameter)
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag EventTag;
};

/**
 * 방 이벤트 전송 StateTree Task
 * GameState에 룸 이벤트를 전송 (즉시 완료)
 */
USTRUCT(meta = (DisplayName = "Send Room Event", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_SendRoomEvent : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_SendRoomEvent_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
