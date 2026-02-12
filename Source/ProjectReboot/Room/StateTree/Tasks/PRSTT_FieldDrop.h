// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueliteTypes.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "PRSTT_FieldDrop.generated.h"

class APRRoomController;

/**
 * 필드 드랍 StateTree Task Instance Data
 */
USTRUCT()
struct FPRSTT_FieldDrop_InstanceData
{
	GENERATED_BODY()

	// 방 컨트롤러 (Context)
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APRRoomController> RoomController = nullptr;

	// 드랍 쿼리 파라미터
	UPROPERTY(EditAnywhere, Category = Parameter)
	FRogueliteQuery DropQuery;

	// 드랍 액터 클래스
	UPROPERTY(EditAnywhere, Category = Parameter)
	TSubclassOf<AActor> DropActorClass;

	// 드랍 간격 (초)
	UPROPERTY(EditAnywhere, Category = Parameter)
	float DropInterval = 5.0f;

	// 누적 경과 시간 (내부 관리)
	float ElapsedTime = 0.f;
};

/**
 * 필드 드랍 StateTree Task
 * 지정 Interval마다 RogueliteSystem에 쿼리하여 드랍 액터 스폰
 */
USTRUCT(meta = (DisplayName = "Field Drop", Category = "Room"))
struct PROJECTREBOOT_API FPRStateTreeTask_FieldDrop : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FPRSTT_FieldDrop_InstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 상태 진입 시 호출
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	// 매 틱 호출
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	// 상태 이탈 시 호출
	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

protected:
	// 드랍 액터 스폰
	void SpawnFieldDrops(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};
