// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"

void UPRAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	PlayerCharacter = Cast<APRPlayerCharacter>(GetOwningActor());
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
	UpdateTurnInPlace();
	UpdateRootYawOffset();
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
	bIsCrouching = PlayerCharacter->IsCrouching();
	bIsSprint = PlayerCharacter->IsSprinting();
	LandState = ELandState::Normal; // TODO: Character 상태 적용
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
	float LookDirection = PlayerCharacter->GetDesiredLookDirection();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	LeanDirection = FMath::FInterpTo(LeanDirection, LookDirection, DeltaSeconds, 10.f);
}

void UPRAnimInstance::UpdateTurnInPlace()
{
    // 커브 값 추출
    const float TurnYawWeight = GetCurveValue(TurnYawWeightCurveName);
    const float CurveRemainingTurnYaw = GetCurveValue(RemainingTurnYawCurveName);
    
    // TurnYawWeight가 0보다 크면 Turn 애니메이션 활성 상태
    bIsTurningInPlace = TurnYawWeight > 0.0f;
    
    // Distance 커브 업데이트 (RemainingTurnYaw 사용)
    LastDistanceCurve = DistanceCurve;
    DistanceCurve = CurveRemainingTurnYaw;
}

void UPRAnimInstance::UpdateRootYawOffset()
{
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
    
	LastMovingRotation = MovingRotation;
	MovingRotation = PlayerCharacter->GetActorRotation();
    
	if (bShouldMove || bIsFalling)
	{
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, DeltaSeconds, RootYawOffsetInterpSpeed);
	}
	else
	{
		FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(MovingRotation, LastMovingRotation);
		RootYawOffset -= DeltaRotator.Yaw;
		RootYawOffset = FRotator::NormalizeAxis(RootYawOffset);
        
		// 최대 Offset 제한 (Turn 시작 전)
		if (!bIsTurningInPlace)
		{
			RootYawOffset = FMath::Clamp(RootYawOffset, -90.0f, 90.0f);
		}
        
		if (bIsTurningInPlace)
		{
			float DeltaDistanceCurve = DistanceCurve - LastDistanceCurve;
			RootYawOffset -= DeltaDistanceCurve;
            
			float AbsRootYawOffset = FMath::Abs(RootYawOffset);
			if (AbsRootYawOffset > TurnStartThreshold)
			{
				float YawExcess = AbsRootYawOffset - TurnStartThreshold;
                
				if (RootYawOffset > 0.0f)
				{
					RootYawOffset -= YawExcess;
				}
				else
				{
					RootYawOffset += YawExcess;
				}
			}
		}
	}
}
