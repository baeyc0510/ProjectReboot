// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_SpawnWave.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"
#include "AbilitySystemComponent.h"

EStateTreeRunStatus FPRStateTreeTask_SpawnWave::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!IsValid(Data.RoomController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SpawnWave: RoomController is invalid"));
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
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SpawnWave: GameState is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	// 킬 카운트 리셋
	GameState->ResetEventCount(TAG_Event_Kill);

	// 웨이브 적 스폰
	Data.SpawnedCount = SpawnWaveEnemies(Context, Data);

	const int32 CurrentWaveIndex = Data.RoomController->GetCurrentWaveIndex();
	const int32 TotalWaves = Data.RoomController->GetSpawnInfo().Waves.Num();
	UE_LOG(LogTemp, Log, TEXT("PRSTT_SpawnWave: Wave %d/%d spawned %d enemies"), CurrentWaveIndex + 1, TotalWaves, Data.SpawnedCount);

	// 즉시 완료 (스폰만 하고 끝)
	return EStateTreeRunStatus::Succeeded;
}

int32 FPRStateTreeTask_SpawnWave::SpawnWaveEnemies(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const
{
	const int32 CurrentWaveIndex = Data.RoomController->GetCurrentWaveIndex();
	const FPRRoomSpawnInfo& SpawnInfo = Data.RoomController->GetSpawnInfo();

	// 웨이브 인덱스 유효성 검사
	if (!SpawnInfo.Waves.IsValidIndex(CurrentWaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SpawnWave: Invalid wave index %d (max: %d)"),
			CurrentWaveIndex, SpawnInfo.Waves.Num() - 1);
		return 0;
	}

	const FPRWaveSpawnInfo& WaveInfo = SpawnInfo.Waves[CurrentWaveIndex];
	UWorld* World = Context.GetWorld();
	const TArray<AActor*>& SpawnPoints = Data.RoomController->EnemySpawnPoints;

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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

	// 적 스폰
	for (const auto& [EnemyClass, Count] : WaveInfo.EnemySpawnMap)
	{
		if (!EnemyClass)
		{
			continue;
		}

		for (int32 i = 0; i < Count; i++)
		{
			FTransform SpawnTransform = GetNextSpawnTransform();
			APREnemyCharacter* SpawnedEnemy = World->SpawnActor<APREnemyCharacter>(
				EnemyClass, SpawnTransform, SpawnParams);
			if (IsValid(SpawnedEnemy))
			{
				// 등장 GameplayCue 실행
				if (UAbilitySystemComponent* ASC = SpawnedEnemy->GetAbilitySystemComponent())
				{
					ASC->ExecuteGameplayCue(TAG_GameplayCue_Character_Appear);
				}
				TotalSpawned++;
			}
		}
	}

	return TotalSpawned;
}
