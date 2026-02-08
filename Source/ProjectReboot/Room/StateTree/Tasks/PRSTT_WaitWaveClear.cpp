// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_WaitWaveClear.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Room/PRRoomController.h"

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

	// RoomController 유효성 검사
	if (!IsValid(Data.RoomController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaitWaveClear: RoomController is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// SpawnInfo에서 웨이브 정보 가져오기
	const int32 CurrentWaveIndex = Data.RoomController->GetCurrentWaveIndex();
	const FPRRoomSpawnInfo& SpawnInfo = Data.RoomController->GetSpawnInfo();
	if (!SpawnInfo.Waves.IsValidIndex(CurrentWaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaitWaveClear: Invalid wave index %d"), CurrentWaveIndex);
		return EStateTreeRunStatus::Failed;
	}

	// 현재 웨이브의 총 적 수 계산
	const FPRWaveSpawnInfo& CurrentWave = SpawnInfo.Waves[CurrentWaveIndex];
	const int32 TargetKillCount = CurrentWave.NormalEnemies.Num() + CurrentWave.EliteEnemies.Num() + CurrentWave.MiniBosses.Num();

	// 처치할 적이 없으면 즉시 완료
	if (TargetKillCount <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: No enemies to kill, wave cleared immediately (Wave %d)"), CurrentWaveIndex);
		Data.RoomController->IncrementWaveIndex();
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
	if (CurrentKillCount >= TargetKillCount)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: Already cleared (%d/%d, Wave %d)"), CurrentKillCount, TargetKillCount, CurrentWaveIndex);
		Data.RoomController->IncrementWaveIndex();
		return EStateTreeRunStatus::Succeeded;
	}

	// 킬 카운트 델리게이트 바인딩 (WeakContext 패턴)
	auto WeakContext = Context.MakeWeakExecutionContext();
	TWeakObjectPtr<APRRoomController> WeakController = Data.RoomController;

	Data.KillCountHandle = GameState->OnEventCountChanged.AddLambda(
		[WeakContext, WeakController, TargetKillCount](const FGameplayTag& Tag, int32 Count)
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

				// 웨이브 인덱스 증가
				if (APRRoomController* RoomController = WeakController.Get())
				{
					RoomController->IncrementWaveIndex();
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

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaitWaveClear: Waiting for %d kills (Wave %d/%d)"), TargetKillCount, CurrentWaveIndex + 1, SpawnInfo.Waves.Num());

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
