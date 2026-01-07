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
	AimPitch = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation).Pitch;
}

void UPRAnimInstance::UpdateLean()
{
	float TurnDirection = PlayerCharacter->GetTurnDirection();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	LeanDirection = FMath::FInterpTo(LeanDirection, TurnDirection, DeltaSeconds, 10.f);
}
