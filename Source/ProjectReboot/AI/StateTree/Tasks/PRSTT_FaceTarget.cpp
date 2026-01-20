// Fill out your copyright notice in the Description page of Project Settings.


#include "PRSTT_FaceTarget.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/KismetMathLibrary.h"

void UPRSTT_FaceTarget::NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::NativeReceivedTick(Context, DeltaTime);
	
	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Controller) || !IsValid(TargetActor))
	{
		return;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	// 타겟 방향 계산
	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);

	// Yaw만 보간
	const FRotator CurrentRotation = Pawn->GetActorRotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, RotationRate / 360.f * 10.f);

	Pawn->SetActorRotation(NewRotation);
}
