#include "PRNPCAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "PRAnimationTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

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

	CachedVelocity = MovementComponent->Velocity;
	CachedForward = OwningPawn->GetActorForwardVector();
	CachedRotation = OwningPawn->GetActorRotation();
	
	if (APREnemyCharacter* EnemyCharacter = Cast<APREnemyCharacter>(OwningPawn.Get()))
	{
		bIsStrafing = EnemyCharacter->IsStrafeMode();
	}
}

void UPRNPCAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	const float CurrentSpeed = CachedVelocity.Size2D();
	bIsMoving = CurrentSpeed > MoveThreshold;

	UpdateDirection();
}

void UPRNPCAnimInstance::UpdateDirection()
{
	FVector Velocity2D = CachedVelocity.GetSafeNormal2D();
	if (CachedVelocity.SizeSquared2D() < 1.0f)
	{
		return;
	}
	
	float Direction =  UKismetAnimationLibrary::CalculateDirection(Velocity2D,CachedRotation);
	
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