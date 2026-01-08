// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectReboot/Character/PlayerCharacter.h"

void UPRAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	PlayerCharacter = Cast<APlayerCharacter>(GetOwningActor());
	if (IsValid(PlayerCharacter))
	{
		CharacterMovement = PlayerCharacter->GetCharacterMovement();
	}
}

void UPRAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!IsValid(PlayerCharacter) || !IsValid(CharacterMovement))
	{
		return;
	}
	
	UpdateVelocity();
	UpdateAcceleration();
	UpdateDirection();
	UpdateAim();
	UpdateLean();
	UpdateFlags();
	
	//TODO: TurnInPlace
}

void UPRAnimInstance::UpdateVelocity()
{
	Velocity = CharacterMovement->Velocity;
	Velocity2D = FVector(Velocity.X, Velocity.Y, 0.0f);
	XYSpeed = Velocity.Size2D();
	
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	LocalVelocity2D = ActorRotation.UnrotateVector(Velocity2D);
}

void UPRAnimInstance::UpdateAcceleration()
{
	Acceleration = CharacterMovement->GetCurrentAcceleration();
	
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	FVector AccelerationXY = FVector(Acceleration.X, Acceleration.Y, 0.0f); 
	LocalAcceleration2D = ActorRotation.UnrotateVector(AccelerationXY);
}
void UPRAnimInstance::UpdateDirection()
{
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	Direction =  UKismetAnimationLibrary::CalculateDirection(Velocity2D,ActorRotation);
	if (Direction >= -70.f && Direction <= 70.f)
	{
		CardinalDirection = ECardinalDirection::Forward;
	}
	else if (Direction > 70.f && Direction <= 110.f)
	{
		CardinalDirection = ECardinalDirection::Right;
	}
	else if (Direction >= -110.f && Direction < -70.f)
	{
		CardinalDirection = ECardinalDirection::Left;
	}
	else
	{
		CardinalDirection = ECardinalDirection::Backward;
	}
	
	F_OrientationAngle = Direction;
	R_OrientationAngle = Direction - 90.f;
	B_OrientationAngle = Direction - 180.f;
	L_OrientationAngle = Direction + 90.f;
}

void UPRAnimInstance::UpdateFlags()
{
	bHasAcceleration = !LocalAcceleration2D.IsNearlyZero();
	bShouldMove = XYSpeed > MoveSpeedThreshold && bHasAcceleration;
	bIsFalling = CharacterMovement->IsFalling();
	
	// TODO: Character 상태 적용
	bIsCrouching = false;
	bIsSprint = false;
	LandState = ELandState::Normal;
}

void UPRAnimInstance::UpdateAim()
{
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);
	
	AimYaw = DeltaRotator.Yaw;
	AimPitch = DeltaRotator.Pitch;
}

void UPRAnimInstance::UpdateLean()
{
	float TurnDirection = PlayerCharacter->GetTurnDirection();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	LeanDirection = FMath::FInterpTo(LeanDirection, TurnDirection, DeltaSeconds, 10.f);
}

void UPRAnimInstance::UpdateTurnInPlace()
{
	// 커브 값 추출
	const float TurnYawWeight = GetCurveValue(TurnYawWeightCurveName);
	const float CurveRemainingTurnYaw = GetCurveValue(RemainingTurnYawCurveName);

	// 이전 값 저장 (델타 계산용)
	PreviousRemainingTurnYaw = RemainingTurnYaw;

	// TurnYawWeight가 1이면 Turn 애니메이션 활성 상태
	const bool bTurnAnimationActive = FMath::IsNearlyEqual(TurnYawWeight, 1.0f, 0.1f);

	if (bTurnAnimationActive)
	{
		// Turn 애니메이션이 재생 중일 때
		if (!bIsTurningInPlace)
		{
			// Turn 시작 시점
			bIsTurningInPlace = true;
			TurnStartYaw = AimYaw;
			RemainingTurnYaw = AimYaw;
		}
		else
		{
			// Turn 진행 중 - 커브 값으로 남은 회전량 업데이트
			const float AnimationTurnProgress = TurnStartYaw - CurveRemainingTurnYaw;
			RemainingTurnYaw = AimYaw - AnimationTurnProgress;
			RemainingTurnYaw = FRotator::NormalizeAxis(RemainingTurnYaw);
		}
	}
	else
	{
		// Turn 애니메이션이 비활성 상태
		if (bIsTurningInPlace)
		{
			// Turn 종료
			bIsTurningInPlace = false;
		}
        
		// Idle 상태에서는 AimYaw가 곧 RemainingTurnYaw
		RemainingTurnYaw = AimYaw;
	}
}

void UPRAnimInstance::ProcessRootYawOffset()
{
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	if (bShouldMove)
	{
		// 이동 중에는 Offset을 0으로 보간
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, DeltaSeconds, RootYawOffsetInterpSpeed);
	}
	else if (bIsTurningInPlace)
	{
		// Turn 중일 때 - 애니메이션 커브의 델타만큼 캐릭터 회전
		const float YawDelta = PreviousRemainingTurnYaw - RemainingTurnYaw;
        
		if (!FMath::IsNearlyZero(YawDelta, 0.01f))
		{
			// 캐릭터 실제 회전
			FRotator NewRotation = PlayerCharacter->GetActorRotation();
			NewRotation.Yaw += YawDelta;
			PlayerCharacter->SetActorRotation(NewRotation);
            
			// 메시 Offset 보정 (캐릭터가 회전한 만큼 반대로)
			RootYawOffset -= YawDelta;
			RootYawOffset = FRotator::NormalizeAxis(RootYawOffset);
		}
	}
	else
	{
		// Idle 상태 - Offset 유지 또는 서서히 해소
		if (FMath::Abs(AimYaw) > TurnStartThreshold)
		{
			// Turn이 곧 시작될 것이므로 Offset 준비
			RootYawOffset = AimYaw;
		}
		else
		{
			// 천천히 0으로 복귀
			RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, DeltaSeconds, RootYawOffsetInterpSpeed * 0.5f);
		}
	}
}
