// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_SendRoomEvent.h"

#include "ProjectReboot/Game/PRGameplayGameState.h"

EStateTreeRunStatus FPRStateTreeTask_SendRoomEvent::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	// EventTag 유효성 검사
	if (!Data.EventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SendRoomEvent: EventTag is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// GameState 가져오기
	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return EStateTreeRunStatus::Failed;
	}

	APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>();
	if (!IsValid(GameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SendRoomEvent: GameState is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// 이벤트 전송
	GameState->SendRoomEvent(Data.EventTag);

	UE_LOG(LogTemp, Log, TEXT("PRSTT_SendRoomEvent: Event sent - %s"), *Data.EventTag.ToString());

	// 즉시 완료
	return EStateTreeRunStatus::Succeeded;
}
