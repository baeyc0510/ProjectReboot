// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_FieldDrop.h"

#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRFieldDropActor.h"

EStateTreeRunStatus FPRStateTreeTask_FieldDrop::EnterState(
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
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_FieldDrop: RoomController is invalid"));
		return EStateTreeRunStatus::Failed;
	}

	if (!Data.DropActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_FieldDrop: DropActorClass is not set"));
		return EStateTreeRunStatus::Failed;
	}

	// 경과 시간 초기화
	Data.ElapsedTime = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_FieldDrop::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime += DeltaTime;

	if (Data.ElapsedTime >= Data.DropInterval)
	{
		SpawnFieldDrops(Context, Data);
		Data.ElapsedTime = 0.f;
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_FieldDrop::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
	{
		return;
	}

	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime = 0.f;
}

void FPRStateTreeTask_FieldDrop::SpawnFieldDrops(
	FStateTreeExecutionContext& Context,
	FInstanceDataType& Data) const
{
	UWorld* World = Context.GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// RogueliteSubsystem 쿼리
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(Data.RoomController);
	if (!IsValid(RogueliteSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRSTT_FieldDrop: RogueliteSubsystem is invalid"));
		return;
	}

	TArray<URogueliteActionData*> QueryResults = RogueliteSubsystem->ExecuteQuery(Data.DropQuery);
	if (QueryResults.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("PRSTT_FieldDrop: No actions found from query"));
		return;
	}

	// 스폰 포인트 목록
	const TArray<AActor*>& SpawnPoints = Data.RoomController->FieldDropSpawnPoints;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 SpawnPointIndex = 0;

	for (URogueliteActionData* ActionData : QueryResults)
	{
		if (!IsValid(ActionData))
		{
			continue;
		}

		// 스폰 위치 결정
		FTransform SpawnTransform;
		if (SpawnPoints.Num() > 0)
		{
			AActor* SpawnPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
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

		// 드랍 액터 스폰
		AActor* SpawnedActor = World->SpawnActor<AActor>(Data.DropActorClass, SpawnTransform, SpawnParams);
		if (IsValid(SpawnedActor))
		{
			// 액션 데이터 설정
			if (APRFieldDropActor* DropActor = Cast<APRFieldDropActor>(SpawnedActor))
			{
				DropActor->SetActionData(ActionData);
			}

			// RoomController에 스폰 액터 등록
			Data.RoomController->RegisterSpawnedActor(SpawnedActor);

			UE_LOG(LogTemp, Log, TEXT("PRSTT_FieldDrop: Spawned drop for action '%s'"),
				*ActionData->DisplayName.ToString());
		}
	}
}
