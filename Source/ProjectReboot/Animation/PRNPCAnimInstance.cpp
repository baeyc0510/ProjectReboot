#include "PRNPCAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "PRAnimationTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPRNPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
	if (IsValid(OwningPawn.Get()))
	{
		if (ACharacter* Character = Cast<ACharacter>(OwningPawn.Get()))
		{
			MovementComponent = Character->GetCharacterMovement();
		}
	}
}

void UPRNPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwningPawn.Get()) || !IsValid(MovementComponent.Get()))
	{
		return;
	}

	// 데이터 캐싱
	CachedVelocity = MovementComponent->Velocity;
	CachedForward = OwningPawn->GetActorForwardVector();
	CachedRotation = OwningPawn->GetActorRotation();
	bCachedOrientToMovement = MovementComponent->bOrientRotationToMovement;
}

void UPRNPCAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	const float CurrentSpeed = CachedVelocity.Size2D();
	bIsMoving = CurrentSpeed > MoveThreshold;

	UpdateDirection();

	// Strafe 상태
	bIsStrafing = !bCachedOrientToMovement;
}

void UPRNPCAnimInstance::UpdateDirection()
{
	FRotator ActorRotation = CachedRotation;
	FVector Velocity2D = CachedVelocity.GetSafeNormal2D();
	float Direction =  UKismetAnimationLibrary::CalculateDirection(Velocity2D,ActorRotation);
	
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