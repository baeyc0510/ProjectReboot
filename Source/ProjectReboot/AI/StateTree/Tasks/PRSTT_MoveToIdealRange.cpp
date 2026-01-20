#include "PRSTT_MoveToIdealRange.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "Tasks/AITask_MoveTo.h"

void UPRSTT_MoveToIdealRange::OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller) || !IsValid(TargetActor))
	{
		FinishTask(false);
		return;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		FinishTask(false);
		return;
	}

	CachedController = Controller;

	// 이상적 거리 위치 계산
	const FVector Destination = CalculateIdealLocation(Pawn);

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(Destination);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetCanStrafe(bCanStrafe);

	// UAITask_MoveTo 생성 및 설정 (엔진의 PrepareMoveToTask 로직)
	if (!MoveToTask)
	{
		MoveToTask = UAITask::NewAITask<UAITask_MoveTo>(*Controller, *Controller);
	}
	MoveToTask->SetUp(Controller, MoveReq);

	// 3. 태스크 활성화
	if (MoveToTask->IsActive())
	{
		MoveToTask->ConditionalPerformMove();
	}
	else
	{
		MoveToTask->ReadyForActivation();
	}

	// 즉시 종료 확인 (이미 도착한 경우 등)
	if (MoveToTask->GetState() == EGameplayTaskState::Finished)
	{
		FinishTask(MoveToTask->WasMoveSuccessful());
	}
}

void UPRSTT_MoveToIdealRange::OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (IsValid(CachedController.Get()))
	{
		CachedController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
		CachedController->StopMovement();
	}
	CachedController.Reset();
	
	if (IsValid(MoveToTask))
	{
		MoveToTask->EndTask();
	}
	MoveToTask = nullptr;
}

void UPRSTT_MoveToIdealRange::NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::NativeReceivedTick(Context, DeltaTime);
	
	if (MoveToTask)
	{
		if (MoveToTask->GetState() == EGameplayTaskState::Finished)
		{
			FinishTask(MoveToTask->WasMoveSuccessful());
		}
	}
}

FVector UPRSTT_MoveToIdealRange::CalculateIdealLocation(APawn* Pawn) const
{
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal2D();

	// 타겟에서 이상적 거리만큼 떨어진 위치
	FVector IdealLocation = TargetLocation - DirectionToTarget * IdealRange;

	// NavMesh 위로 보정
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(IdealLocation, NavLocation))
		{
			IdealLocation = NavLocation.Location;
		}
	}

	return IdealLocation;
}