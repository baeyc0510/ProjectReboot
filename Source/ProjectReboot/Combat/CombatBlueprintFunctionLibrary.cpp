// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatBlueprintFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

namespace
{
	void FilterHitResultsByTargetClasses(TArray<FHitResult>& HitResults, const TArray<TSubclassOf<AActor>>& TargetClasses)
	{
		if (TargetClasses.Num() <= 0)
		{
			return;
		}

		TArray<FHitResult> FilteredHits;
		FilteredHits.Reserve(HitResults.Num());

		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!IsValid(HitActor))
			{
				continue;
			}

			bool bMatchedTargetClass = false;
			for (const TSubclassOf<AActor>& TargetClass : TargetClasses)
			{
				if (IsValid(TargetClass) && HitActor->IsA(TargetClass))
				{
					bMatchedTargetClass = true;
					break;
				}
			}

			if (bMatchedTargetClass)
			{
				FilteredHits.Add(HitResult);
			}
		}

		HitResults = MoveTemp(FilteredHits);
	}

	void TrimHitResultsByMaxCount(TArray<FHitResult>& HitResults, int32 MaxHitCount)
	{
		if (MaxHitCount <= 0 || HitResults.Num() <= MaxHitCount)
		{
			return;
		}

		HitResults.SetNum(MaxHitCount);
	}
}

EPRHitDirection UCombatBlueprintFunctionLibrary::GetHitDirectionFromHitResult(const FHitResult& HitResult, const AActor* HitActor)
{
	if (!IsValid(HitActor))
	{
		return EPRHitDirection::Front;
	}

	// 피격 방향 계산 (ImpactPoint -> TraceStart)
	FVector HitDirection = (HitResult.TraceStart - HitResult.ImpactPoint).GetSafeNormal();
	
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

	FVector HitDirection = (Instigator->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();
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

/*~ Trace ~*/

bool UCombatBlueprintFunctionLibrary::SphereSweepTraceByStartEnd(
	const UObject* WorldContextObject,
	const FVector& Start,
	const FVector& End,
	const FVector& Direction,
	float Radius,
	TEnumAsByte<ECollisionChannel> TraceChannel,
	const FCollisionQueryParams& QueryParams,
	TArray<FHitResult>& OutHits,
	bool bDrawDebug,
	float DebugDrawTime,
	const TArray<TSubclassOf<AActor>>& TargetClasses,
	bool bUseCylinderFilter,
	float CylinderHalfHeight,
	int32 MaxHitCount)
{
	OutHits.Reset();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!IsValid(World))
	{
		return false;
	}

	const bool bHit = World->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(FMath::Max(0.0f, Radius)),
		QueryParams
	);

	if (bDrawDebug)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, Start, End, DebugColor, false, DebugDrawTime, 0, 1.0f);
		DrawDebugSphere(World, Start, Radius, 12, DebugColor, false, DebugDrawTime);
		DrawDebugSphere(World, End, Radius, 12, DebugColor, false, DebugDrawTime);
		DrawDebugDirectionalArrow(World, Start, Start + Direction.GetSafeNormal() * 50.0f, 20.0f, DebugColor, false, DebugDrawTime);
	}

	if (!bHit)
	{
		return false;
	}

	if (bUseCylinderFilter)
	{
		const FVector CylinderCenter = (Start + End) * 0.5f;
		TArray<FHitResult> FilteredHits;
		FilteredHits.Reserve(OutHits.Num());

		for (const FHitResult& HitResult : OutHits)
		{
			if (IsInsideCylinder(HitResult.Location, CylinderCenter, Radius, CylinderHalfHeight))
			{
				FilteredHits.Add(HitResult);
			}
		}

		OutHits = MoveTemp(FilteredHits);
	}

	FilterHitResultsByTargetClasses(OutHits, TargetClasses);
	TrimHitResultsByMaxCount(OutHits, MaxHitCount);

	if (OutHits.Num() <= 0)
	{
		return false;
	}

	NotifyCombatInterfaceHit(OutHits);

	return true;
}

bool UCombatBlueprintFunctionLibrary::BoxSweepTraceByStartEnd(
	const UObject* WorldContextObject,
	const FVector& Start,
	const FVector& End,
	const FVector& Direction,
	const FVector& HalfExtent,
	const FRotator& BoxRotation,
	TEnumAsByte<ECollisionChannel> TraceChannel,
	const FCollisionQueryParams& QueryParams,
	TArray<FHitResult>& OutHits,
	bool bDrawDebug,
	float DebugDrawTime,
	const TArray<TSubclassOf<AActor>>& TargetClasses,
	int32 MaxHitCount)
{
	OutHits.Reset();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!IsValid(World))
	{
		return false;
	}

	const bool bHit = World->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		BoxRotation.Quaternion(),
		TraceChannel,
		FCollisionShape::MakeBox(HalfExtent.ComponentMax(FVector::ZeroVector)),
		QueryParams
	);

	if (bDrawDebug)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, Start, End, DebugColor, false, DebugDrawTime, 0, 1.0f);
		DrawDebugBox(World, Start, HalfExtent, BoxRotation.Quaternion(), DebugColor, false, DebugDrawTime);
		DrawDebugBox(World, End, HalfExtent, BoxRotation.Quaternion(), DebugColor, false, DebugDrawTime);
		DrawDebugDirectionalArrow(World, Start, Start + Direction.GetSafeNormal() * 50.0f, 20.0f, DebugColor, false, DebugDrawTime);
	}

	if (!bHit)
	{
		return false;
	}

	FilterHitResultsByTargetClasses(OutHits, TargetClasses);
	TrimHitResultsByMaxCount(OutHits, MaxHitCount);

	if (OutHits.Num() <= 0)
	{
		return false;
	}

	NotifyCombatInterfaceHit(OutHits);

	return true;
}

bool UCombatBlueprintFunctionLibrary::IsInsideCylinder(const FVector& TestPoint, const FVector& CylinderCenter, float Radius, float HalfHeight)
{
	const FVector Delta = TestPoint - CylinderCenter;
	if (FMath::Abs(Delta.Z) > HalfHeight)
	{
		return false;
	}

	const float Distance2DSquared = Delta.X * Delta.X + Delta.Y * Delta.Y;
	return Distance2DSquared <= FMath::Square(Radius);
}

bool UCombatBlueprintFunctionLibrary::TraceBySettings(
	const UObject* WorldContextObject,
	const FVector& Start,
	const FVector& End,
	const FVector& Direction,
	const FPRTraceSettings& TraceSettings,
	const FCollisionQueryParams& QueryParams,
	TArray<FHitResult>& OutHits,
	const FRotator& BoxRotation)
{
	OutHits.Reset();

	bool bHit = false;
	if (TraceSettings.TraceShape == EPRTraceShape::Sphere)
	{
		bHit = SphereSweepTraceByStartEnd(
			WorldContextObject,
			Start,
			End,
			Direction,
			TraceSettings.TraceRadius,
			TraceSettings.TraceChannel,
			QueryParams,
			OutHits,
			TraceSettings.bDrawDebugTrace,
			TraceSettings.DebugDrawTime,
			TraceSettings.TraceTargetClasses,
			TraceSettings.bUseCylinderFilter,
			TraceSettings.CylinderHalfHeight,
			TraceSettings.MaxHitCount
		);
	}
	else if (TraceSettings.TraceShape == EPRTraceShape::Box)
	{
		bHit = BoxSweepTraceByStartEnd(
			WorldContextObject,
			Start,
			End,
			Direction,
			TraceSettings.TraceBoxHalfExtent,
			BoxRotation,
			TraceSettings.TraceChannel,
			QueryParams,
			OutHits,
			TraceSettings.bDrawDebugTrace,
			TraceSettings.DebugDrawTime,
			TraceSettings.TraceTargetClasses,
			TraceSettings.MaxHitCount
		);
	}

	return bHit;
}

void UCombatBlueprintFunctionLibrary::NotifyCombatInterfaceHit(const TArray<FHitResult>& HitResults)
{
	TSet<AActor*> ProcessedActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		ProcessedActors.Add(HitActor);

		if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(HitActor))
		{
			CombatInterface->OnHit(HitResult);
		}
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
