// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_WaitEventCount.h"

#include "ProjectReboot/Game/PRGameplayGameState.h"

EStateTreeRunStatus FPRStateTreeTask_WaitEventCount::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.bCompleted = false;

	// EventTag 유효성 검사
	if (!Data.EventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaitEventCount: EventTag is invalid"));
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
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaitEventCount: GameState is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// 현재 카운트 확인 (이미 목표 달성했을 수 있음)
	const int32 CurrentCount = GameState->GetEventCount(Data.EventTag);
	if (CurrentCount >= Data.TargetCount)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Already reached target (%d/%d, Tag: %s)"),
			CurrentCount, Data.TargetCount, *Data.EventTag.ToString());

		// OnCompletionEvents 발생
		for (const FGameplayTag& CompletionEvent : Data.OnCompletionEvents)
		{
			if (CompletionEvent.IsValid())
			{
				GameState->SendRoomEvent(CompletionEvent);
				UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Sent completion event - %s"), *CompletionEvent.ToString());
			}
		}

		return EStateTreeRunStatus::Succeeded;
	}

	// 델리게이트 바인딩 (WeakContext 패턴)
	auto WeakContext = Context.MakeWeakExecutionContext();

	Data.EventHandle = GameState->OnEventCountChanged.AddLambda(
		[WeakContext, TargetTag = Data.EventTag, TargetCount = Data.TargetCount, CompletionEvents = Data.OnCompletionEvents]
		(const FGameplayTag& Tag, int32 Count)
		{
			if (Tag != TargetTag)
			{
				return;
			}

			if (Count >= TargetCount)
			{
				// Strong Context로 변환
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
				if (!StrongContext.IsValid())
				{
					return;
				}

				// Instance Data 업데이트
				if (FPRSTT_WaitEventCount_InstanceData* LambdaData = StrongContext.GetInstanceDataPtr<FPRSTT_WaitEventCount_InstanceData>())
				{
					LambdaData->bCompleted = true;
				}

				// OnCompletionEvents 발생
				UWorld* World = StrongContext.GetOwner()->GetWorld();
				if (IsValid(World))
				{
					if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
					{
						for (const FGameplayTag& CompletionEvent : CompletionEvents)
						{
							if (CompletionEvent.IsValid())
							{
								GameState->SendRoomEvent(CompletionEvent);
								UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Sent completion event - %s"), *CompletionEvent.ToString());
							}
						}
					}
				}

				UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Target reached (%d/%d, Tag: %s)"),
					Count, TargetCount, *TargetTag.ToString());
			}
		}
	);

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Waiting for %d counts (Tag: %s)"), Data.TargetCount, *Data.EventTag.ToString());

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_WaitEventCount::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.bCompleted)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_WaitEventCount::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);

	// 델리게이트 해제
	if (Data.EventHandle.IsValid())
	{
		UWorld* World = Context.GetWorld();
		if (IsValid(World))
		{
			if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
			{
				GameState->OnEventCountChanged.Remove(Data.EventHandle);
			}
		}
		Data.EventHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitEventCount: Task ended (Tag: %s)"), *Data.EventTag.ToString());
}
