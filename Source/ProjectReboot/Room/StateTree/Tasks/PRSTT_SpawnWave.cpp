// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_SpawnWave.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "AbilitySystemComponent.h"

EStateTreeRunStatus FPRStateTreeTask_SpawnWave::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	// 이미 스폰 진행 중이면 재구축하지 않음 (Sustained 재진입 방어)
	if (Data.bSpawning)
	{
		return EStateTreeRunStatus::Running;
	}

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

	// 스폰 큐 준비
	const int32 CurrentWaveIndex = Data.RoomController->GetCurrentWaveIndex();
	const FPRRoomSpawnInfo& SpawnInfo = Data.RoomController->GetSpawnInfo();

	if (!SpawnInfo.Waves.IsValidIndex(CurrentWaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_SpawnWave: Invalid wave index %d (max: %d)"),
			CurrentWaveIndex, SpawnInfo.Waves.Num() - 1);
		return EStateTreeRunStatus::Succeeded;
	}

	const FPRWaveSpawnInfo& WaveInfo = SpawnInfo.Waves[CurrentWaveIndex];
	Data.SpawnQueue.Empty();
	Data.SpawnedCount = 0;
	Data.SpawnPointIndex = 0;
	Data.TotalToSpawn = 0;

	for (const auto& [EnemyClass, Count] : WaveInfo.EnemySpawnMap)
	{
		if (EnemyClass && Count > 0)
		{
			FPRSpawnQueueEntry Entry;
			Entry.EnemyClass = EnemyClass;
			Entry.RemainingCount = Count;
			Data.SpawnQueue.Add(Entry);
			Data.TotalToSpawn += Count;
		}
	}

	const int32 TotalWaves = SpawnInfo.Waves.Num();
	UE_LOG(LogTemp, Log, TEXT("PRSTT_SpawnWave: Wave %d/%d queued %d enemies (SpawnPerBatch: %d, Interval: %.2fs)"),
		CurrentWaveIndex + 1, TotalWaves, Data.TotalToSpawn, Data.SpawnPerBatch, Data.SpawnInterval);

	// 큐가 비어있으면 즉시 완료
	if (Data.TotalToSpawn == 0)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	Data.ElapsedTime = Data.SpawnInterval; // 첫 Tick에서 즉시 스폰 시작
	Data.bSpawning = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_SpawnWave::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	Data.ElapsedTime += DeltaTime;
	if (Data.ElapsedTime < Data.SpawnInterval)
	{
		return EStateTreeRunStatus::Running;
	}

	// 간격 도달 시 누적 시간 리셋 후 배치 스폰
	Data.ElapsedTime = 0.f;
	const int32 SpawnThisBatch = FMath::Max(1, Data.SpawnPerBatch);

	for (int32 i = 0; i < SpawnThisBatch; i++)
	{
		if (!SpawnNextEnemy(Context, Data))
		{
			// 큐 소진 → 스폰 완료
			UE_LOG(LogTemp, Log, TEXT("PRSTT_SpawnWave: Spawn complete (%d enemies)"), Data.SpawnedCount);
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_SpawnWave::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.SpawnQueue.Empty();
	Data.SpawnedCount = 0;
	Data.SpawnPointIndex = 0;
	Data.TotalToSpawn = 0;
	Data.ElapsedTime = 0.f;
	Data.bSpawning = false;
}

bool FPRStateTreeTask_SpawnWave::SpawnNextEnemy(
	FStateTreeExecutionContext& Context,
	FInstanceDataType& Data) const
{
	// 큐에서 남은 항목 찾기
	FPRSpawnQueueEntry* ActiveEntry = nullptr;
	for (FPRSpawnQueueEntry& Entry : Data.SpawnQueue)
	{
		if (Entry.RemainingCount > 0)
		{
			ActiveEntry = &Entry;
			break;
		}
	}

	if (!ActiveEntry)
	{
		return false;
	}

	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	// 스폰 위치 결정
	const TArray<AActor*>& SpawnPoints = Data.RoomController->EnemySpawnPoints;
	FTransform SpawnTransform;
	if (SpawnPoints.Num() > 0)
	{
		AActor* SpawnPoint = SpawnPoints[Data.SpawnPointIndex % SpawnPoints.Num()];
		Data.SpawnPointIndex++;
		if (IsValid(SpawnPoint))
		{
			SpawnTransform = SpawnPoint->GetActorTransform();
		}
		else
		{
			SpawnTransform = Data.RoomController->GetActorTransform();
		}
	}
	else
	{
		SpawnTransform = Data.RoomController->GetActorTransform();
	}

	// 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	APREnemyCharacter* SpawnedEnemy = World->SpawnActor<APREnemyCharacter>(
		ActiveEntry->EnemyClass, SpawnTransform, SpawnParams);

	if (IsValid(SpawnedEnemy))
	{
		// 등장 GameplayCue 실행
		if (UAbilitySystemComponent* ASC = SpawnedEnemy->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(SpawnedEnemy, SpawnedEnemy);
			ASC->ExecuteGameplayCue(TAG_GameplayCue_Character_Appear, EffectContext);
		}
		// 스폰 액터 등록
		Data.RoomController->RegisterSpawnedActor(SpawnedEnemy);
		Data.SpawnedCount++;
	}
	// 스폰 실패
	else
	{
		if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
		{
			GameState->AddEventCount(TAG_Event_Kill);
			// TODO: Enemy 타입 분기?
		}
	}

	ActiveEntry->RemainingCount--;
	return true;
}
