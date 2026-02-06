// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_WaveSpawn.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

EStateTreeRunStatus FPRStateTreeTask_WaveSpawn::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!IsValid(Data.RoomController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaveSpawn: RoomController is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return EStateTreeRunStatus::Failed;
	}

	APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>();
	if (!IsValid(GameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaveSpawn: GameState is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// 킬 카운트 리셋
	GameState->ResetEventCount(TAG_Event_Kill);

	// 웨이브 적 스폰
	Data.SpawnedCount = SpawnWaveEnemies(Context, Data);

	if (Data.SpawnedCount <= 0)
	{
		// 스폰할 적이 없으면 즉시 클리어
		GameState->SendRoomEvent(TAG_Event_Wave_Cleared);
		return EStateTreeRunStatus::Succeeded;
	}

	// 킬 카운트 델리게이트 바인딩 (WeakContext 패턴)
	auto WeakContext = Context.MakeWeakExecutionContext();

	Data.KillCountHandle = GameState->OnEventCountChanged.AddLambda(
		[WeakContext, SpawnedCount = Data.SpawnedCount](const FGameplayTag& Tag, int32 Count)
		{
			if (Tag != TAG_Event_Kill)
			{
				return;
			}

			if (Count >= SpawnedCount)
			{
				// Strong Context로 변환
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();
				if (!StrongContext.IsValid())
				{
					return;
				}

				// GameState에서 웨이브 클리어 이벤트 전송
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

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaveSpawn: Wave %d started, spawned %d enemies"), Data.WaveIndex, Data.SpawnedCount);

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_WaveSpawn::ExitState(
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

	UE_LOG(LogTemp, Log, TEXT("PRSTT_WaveSpawn: Wave %d ended"), Data.WaveIndex);
}

int32 FPRStateTreeTask_WaveSpawn::SpawnWaveEnemies(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const
{
	const FPRRoomSpawnInfo& SpawnInfo = Data.RoomController->GetSpawnInfo();

	// 웨이브 인덱스 유효성 검사
	if (!SpawnInfo.Waves.IsValidIndex(Data.WaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_WaveSpawn: Invalid wave index %d (max: %d)"),
			Data.WaveIndex, SpawnInfo.Waves.Num() - 1);
		return 0;
	}

	const FPRWaveSpawnInfo& WaveInfo = SpawnInfo.Waves[Data.WaveIndex];
	UWorld* World = Context.GetWorld();
	const TArray<AActor*>& SpawnPoints = Data.RoomController->EnemySpawnPoints;

	int32 TotalSpawned = 0;
	int32 SpawnPointIndex = 0;

	// 스폰 위치 선택 헬퍼
	auto GetNextSpawnTransform = [&]() -> FTransform
	{
		if (SpawnPoints.Num() > 0)
		{
			AActor* SpawnPoint = SpawnPoints[SpawnPointIndex % SpawnPoints.Num()];
			SpawnPointIndex++;
			if (IsValid(SpawnPoint))
			{
				return SpawnPoint->GetActorTransform();
			}
		}
		// 스폰 포인트가 없으면 RoomController 위치 사용
		return Data.RoomController->GetActorTransform();
	};

	// 일반 적 스폰
	for (const TSubclassOf<APREnemyCharacter>& EnemyClass : WaveInfo.NormalEnemies)
	{
		if (!EnemyClass)
		{
			continue;
		}

		FTransform SpawnTransform = GetNextSpawnTransform();
		APREnemyCharacter* SpawnedEnemy = World->SpawnActor<APREnemyCharacter>(EnemyClass, SpawnTransform);
		if (IsValid(SpawnedEnemy))
		{
			TotalSpawned++;
		}
	}

	// 엘리트 적 스폰
	for (const TSubclassOf<APREnemyCharacter>& EnemyClass : WaveInfo.EliteEnemies)
	{
		if (!EnemyClass)
		{
			continue;
		}

		FTransform SpawnTransform = GetNextSpawnTransform();
		APREnemyCharacter* SpawnedEnemy = World->SpawnActor<APREnemyCharacter>(EnemyClass, SpawnTransform);
		if (IsValid(SpawnedEnemy))
		{
			TotalSpawned++;
		}
	}

	// 미니보스 스폰
	for (const TSubclassOf<APREnemyCharacter>& EnemyClass : WaveInfo.MiniBosses)
	{
		if (!EnemyClass)
		{
			continue;
		}

		FTransform SpawnTransform = GetNextSpawnTransform();
		APREnemyCharacter* SpawnedEnemy = World->SpawnActor<APREnemyCharacter>(EnemyClass, SpawnTransform);
		if (IsValid(SpawnedEnemy))
		{
			TotalSpawned++;
		}
	}

	return TotalSpawned;
}
