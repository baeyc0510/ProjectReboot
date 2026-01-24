#include "PRSTT_FlyToTarget_Deprecated.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/PawnMovementComponent.h"

void UPRSTT_FlyToTarget::OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
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

    CachedPawn = Pawn;
    CachedDestination = CalculateDestination(Pawn);

    // 이미 도착한 경우
    const float DistanceToGoal = FVector::Dist(Pawn->GetActorLocation(), CachedDestination);
    if (DistanceToGoal <= AcceptanceRadius)
    {
        FinishTask(true);
    }
}

void UPRSTT_FlyToTarget::OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    if (IsValid(CachedPawn.Get()))
    {
        if (UPawnMovementComponent* MovementComp = CachedPawn->GetMovementComponent())
        {
            MovementComp->StopMovementImmediately();
        }
    }
    
    CachedPawn.Reset();
}

void UPRSTT_FlyToTarget::NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::NativeReceivedTick(Context, DeltaTime);

    APawn* Pawn = CachedPawn.Get();
    if (!IsValid(Pawn) || !IsValid(TargetActor))
    {
        FinishTask(false);
        return;
    }

    // 목표 위치 갱신 (타겟이 움직일 수 있으므로)
    CachedDestination = CalculateDestination(Pawn);

    const FVector CurrentLocation = Pawn->GetActorLocation();
    const FVector ToDestination = CachedDestination - CurrentLocation;
    const float DistanceToGoal = ToDestination.Size();

    // 도착 판정
    if (DistanceToGoal <= AcceptanceRadius)
    {
        FinishTask(true);
        return;
    }

    // 이동 방향 계산 및 입력 적용
    const FVector MoveDirection = ToDestination.GetSafeNormal();
    Pawn->AddMovementInput(MoveDirection, 1.f);
}

FVector UPRSTT_FlyToTarget::CalculateDestination(APawn* Pawn) const
{
    const FVector PawnLocation = Pawn->GetActorLocation();
    const FVector TargetLocation = TargetActor->GetActorLocation();
    
    // 2D 방향으로 이상적 거리 계산
    const FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal2D();
    FVector Destination = TargetLocation - DirectionToTarget * IdealRange;
    
    // 높이 오프셋 적용
    Destination.Z = TargetLocation.Z + HeightOffset;
    
    return Destination;
}