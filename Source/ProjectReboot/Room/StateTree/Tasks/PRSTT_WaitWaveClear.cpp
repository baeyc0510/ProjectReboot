// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_WaitWaveClear.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"

EStateTreeRunStatus FPRStateTreeTask_WaitWaveClear::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.bTaskCompleted = false;

	// 처치할 적이 없으면 즉시 완료
	if (Data.TargetKillCount <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: No enemies to kill, wave cleared immediately"));
		return EStateTreeRunStatus::Succeeded;
	}

	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return EStateTreeRunStatus::Failed;
	}

	APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>();
	if (!IsValid(GameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaitWaveClear: GameState is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// 현재 킬 카운트 확인 (이미 도달했을 수 있음)
	const int32 CurrentKillCount = GameState->GetEventCount(TAG_Event_Kill);
	if (CurrentKillCount >= Data.TargetKillCount)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: Already cleared (%d/%d)"), CurrentKillCount, Data.TargetKillCount);
		return EStateTreeRunStatus::Succeeded;
	}

	// 킬 카운트 델리게이트 바인딩 (WeakContext 패턴)
	auto WeakContext = Context.MakeWeakExecutionContext();
	const int32 TargetKillCount = Data.TargetKillCount;

	Data.KillCountHandle = GameState->OnEventCountChanged.AddLambda(
		[WeakContext, TargetKillCount](const FGameplayTag& Tag, int32 Count)
		{
			if (Tag != TAG_Event_Kill)
			{
				return;
			}

			if (Count >= TargetKillCount)
			{
				// Strong Context로 변환
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
				if (!StrongContext.IsValid())
				{
					return;
				}

				// Instance Data 업데이트
				if (FPRSTT_WaitWaveClear_InstanceData* LambdaData = StrongContext.GetInstanceDataPtr<FPRSTT_WaitWaveClear_InstanceData>())
				{
					LambdaData->bTaskCompleted = true;
				}

				// 웨이브 클리어 이벤트 전송
				UWorld* World = StrongContext.GetOwner()->GetWorld();
				if (IsValid(World))
				{
					if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
					{
						GameState->SendRoomEvent(TAG_Event_Wave_Cleared);
					}
				}
			}
		}
	);

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: Waiting for %d kills"), Data.TargetKillCount);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_WaitWaveClear::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.bTaskCompleted)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_WaitWaveClear::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);

	// 델리게이트 해제
	if (Data.KillCountHandle.IsValid())
	{
		UWorld* World = Context.GetWorld();
		if (IsValid(World))
		{
			if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
			{
				GameState->OnEventCountChanged.Remove(Data.KillCountHandle);
			}
		}
		Data.KillCountHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: Wave clear wait ended"));
}
