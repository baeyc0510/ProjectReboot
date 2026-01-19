// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatBlueprintFunctionLibrary.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

EPRHitDirection UCombatBlueprintFunctionLibrary::GetHitDirectionFromHitResult(const FHitResult& HitResult, const AActor* HitActor)
{
	if (!IsValid(HitActor))
	{
		return EPRHitDirection::Front;
	}

	// 피격 방향 계산 (TraceStart 로부터 ImpactPoint)
	FVector HitDirection = (HitResult.ImpactPoint - HitResult.TraceStart).GetSafeNormal();
	
	// 월드 to 로컬 방향 변환
	FVector LocalDirection = HitActor->GetActorRotation().UnrotateVector(HitDirection);

	// X: Forward/Back, Y: Right/Left
	if (FMath::Abs(LocalDirection.X) >= FMath::Abs(LocalDirection.Y))
	{
		return LocalDirection.X >= 0.0f ? EPRHitDirection::Front : EPRHitDirection::Back;
	}
	else
	{
		return LocalDirection.Y >= 0.0f ? EPRHitDirection::Right : EPRHitDirection::Left;
	}
}

EPRHitDirection UCombatBlueprintFunctionLibrary::GetHitDirectionFromInstigator(const AActor* Instigator, const AActor* HitActor)
{
	if (!IsValid(Instigator) || !IsValid(HitActor))
	{
		return EPRHitDirection::Front;
	}

	FVector HitDirection = (HitActor->GetActorLocation() - Instigator->GetActorLocation()).GetSafeNormal();
	FVector LocalDirection = HitActor->GetActorRotation().UnrotateVector(HitDirection);

	if (FMath::Abs(LocalDirection.X) >= FMath::Abs(LocalDirection.Y))
	{
		return LocalDirection.X >= 0.0f ? EPRHitDirection::Front : EPRHitDirection::Back;
	}
	else
	{
		return LocalDirection.Y >= 0.0f ? EPRHitDirection::Right : EPRHitDirection::Left;
	}
}

/*~ Ragdoll ~*/

void UCombatBlueprintFunctionLibrary::EnableRagdoll(ACharacter* Character, const FName& RagdollCollisionProfile)
{
	if (!IsValid(Character))
	{
		return;
	}

	// 1. 캡슐 콜리전 비활성화
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 2. 메시 물리 활성화
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		Mesh->SetCollisionProfileName(RagdollCollisionProfile);
        
		// 미끄러짐 방지를 위한 감쇄 값 설정
		Mesh->SetLinearDamping(0.9f);
		Mesh->SetAngularDamping(0.9f);

		Mesh->SetSimulatePhysics(true);
		Mesh->WakeAllRigidBodies();
	}

	// 3. 이동 비활성화
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
		MovementComp->SetComponentTickEnabled(false);
	}
}

void UCombatBlueprintFunctionLibrary::EnableRagdollWithImpulse(
	ACharacter* Character,
	const FVector& Impulse,
	const FName& BoneName,
	const FName& RagdollCollisionProfile)
{
	EnableRagdoll(Character, RagdollCollisionProfile);

	if (!IsValid(Character))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}

	// Impulse 적용
	if (BoneName.IsNone())
	{
		Mesh->AddImpulse(Impulse, NAME_None, true);
	}
	else
	{
		Mesh->AddImpulseToAllBodiesBelow(Impulse, BoneName, true);
	}
}

void UCombatBlueprintFunctionLibrary::DisableRagdoll(ACharacter* Character)
{
	if (!IsValid(Character))
	{
		return;
	}

	// 메시 물리 비활성화
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetAllBodiesSimulatePhysics(false);
		Mesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	}

	// 캡슐 콜리전 복원
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	}

	// 이동 복원
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
		MovementComp->SetComponentTickEnabled(true);
	}
}

void UCombatBlueprintFunctionLibrary::FreezeRagdoll(ACharacter* Character)
{
	if (!IsValid(Character))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}

	// 모든 바디 Sleep 처리
	Mesh->PutAllRigidBodiesToSleep();
}
