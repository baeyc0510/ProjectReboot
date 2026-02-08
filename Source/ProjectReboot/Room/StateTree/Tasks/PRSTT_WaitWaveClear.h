// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_WaitWaveClear.generated.h"

class APRRoomController;

/**
 * 웨이브 클리어 대기 Task Instance Data
 */
USTRUCT()
struct FPRSTT_WaitWaveClear_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;

	// 웨이브 클리어 이벤트 수신 여부 (런타임)
	bool bTaskCompleted = false;

	// 델리게이트 핸들
	FDelegateHandle KillCountHandle;
};

/**
 * 웨이브 클리어 대기 StateTree Task
 * 지정된 수의 적이 처치될 때까지 대기
 */
USTRUCT(meta = (DisplayName = "Wait Wave Clear", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_WaitWaveClear : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_WaitWaveClear_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
