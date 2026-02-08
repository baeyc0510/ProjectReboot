// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTT_Patrol.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "Tasks/AITask_MoveTo.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

EStateTreeRunStatus FPRStateTreeTask_Patrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller) || !IsValid(InstanceData.EnemyCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CachedController = Controller;
	InstanceData.bIsWaiting = false;
	InstanceData.WaitElapsedTime = 0.f;

	// 첫 배회 위치로 이동 시작
	StartMoveToNextLocation(Context, InstanceData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPRStateTreeTask_Patrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 대기 중인 경우
	if (InstanceData.bIsWaiting)
	{
		InstanceData.WaitElapsedTime += DeltaTime;
		if (InstanceData.WaitElapsedTime >= InstanceData.WaitTimeAtPoint)
		{
			// 대기 완료, 다음 위치로 이동
			InstanceData.bIsWaiting = false;
			InstanceData.WaitElapsedTime = 0.f;
			StartMoveToNextLocation(Context, InstanceData);
		}
		return EStateTreeRunStatus::Running;
	}

	// 이동 중인 경우
	if (InstanceData.MoveToTask)
	{
		if (InstanceData.MoveToTask->GetState() == EGameplayTaskState::Finished)
		{
			bool bWasMoveSuccessful = InstanceData.MoveToTask->WasMoveSuccessful();
			if (bWasMoveSuccessful)
			{
				// 도착 성공, 대기 시작
				InstanceData.bIsWaiting = true;
				InstanceData.WaitElapsedTime = 0.f;
			}
			else
			{
				// 이동 실패, 새로운 위치로 재시도
				StartMoveToNextLocation(Context, InstanceData);
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FPRStateTreeTask_Patrol::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (IsValid(InstanceData.CachedController.Get()))
	{
		InstanceData.CachedController->StopMovement();
	}
	InstanceData.CachedController.Reset();

	if (IsValid(InstanceData.MoveToTask))
	{
		InstanceData.MoveToTask->EndTask();
	}
	InstanceData.MoveToTask = nullptr;

	InstanceData.bIsWaiting = false;
	InstanceData.WaitElapsedTime = 0.f;
}

FVector FPRStateTreeTask_Patrol::GenerateRandomPatrolLocation(const FInstanceDataType& Data, UWorld* World) const
{
	if (!IsValid(Data.EnemyCharacter))
	{
		return FVector::ZeroVector;
	}

	const FVector SpawnLocation = Data.EnemyCharacter->GetSpawnLocation();

	// 스폰 위치 중심으로 반경 내 랜덤 위치 생성
	const float RandomAngle = FMath::FRandRange(0.f, 2.f * PI);
	const float RandomDistance = FMath::FRandRange(0.f, Data.PatrolRadius);
	const FVector RandomOffset = FVector(
		FMath::Cos(RandomAngle) * RandomDistance,
		FMath::Sin(RandomAngle) * RandomDistance,
		0.f
	);

	FVector PatrolLocation = SpawnLocation + RandomOffset;

	// NavMesh 위로 보정
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(PatrolLocation, NavLocation, FVector(Data.PatrolRadius, Data.PatrolRadius, 500.f)))
		{
			PatrolLocation = NavLocation.Location;
		}
	}

	return PatrolLocation;
}

void FPRStateTreeTask_Patrol::StartMoveToNextLocation(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const
{
	if (!IsValid(Data.CachedController.Get()))
	{
		return;
	}

	// 랜덤 배회 위치 생성
	const FVector Destination = GenerateRandomPatrolLocation(Data, Context.GetWorld());

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(Destination);
	MoveReq.SetAcceptanceRadius(Data.AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetCanStrafe(false);

	// UAITask_MoveTo 생성 및 설정
	if (!Data.MoveToTask)
	{
		Data.MoveToTask = UAITask::NewAITask<UAITask_MoveTo>(*Data.CachedController.Get(), *Data.CachedController.Get());
	}
	Data.MoveToTask->SetUp(Data.CachedController.Get(), MoveReq);

	// 태스크 활성화
	if (Data.MoveToTask->IsActive())
	{
		Data.MoveToTask->ConditionalPerformMove();
	}
	else
	{
		Data.MoveToTask->ReadyForActivation();
	}
}
