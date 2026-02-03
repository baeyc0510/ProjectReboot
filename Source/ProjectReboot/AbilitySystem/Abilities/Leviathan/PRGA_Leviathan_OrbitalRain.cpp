// Fill out your copyright notice in the Description page of Project Settings.

#include "PRGA_Leviathan_OrbitalRain.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/OrbitalStrike/PROrbitalStrike.h"

UPRGA_Leviathan_OrbitalRain::UPRGA_Leviathan_OrbitalRain()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPRGA_Leviathan_OrbitalRain::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Strike 스폰
	SpawnStrikes();

	// 몽타주 재생 (있으면)
	if (IsValid(CastMontage))
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			CastMontage,
			MontagePlayRate,
			NAME_None,
			false
		);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 없으면 마지막 Strike 생성 완료 후 종료 예약
		const int32 StrikeCount = FMath::RandRange(MinStrikeCount, MaxStrikeCount);
		const float TotalDuration = SpawnInterval * (StrikeCount - 1) + TelegraphDuration + 1.0f;

		FTimerHandle EndTimer;
		GetWorld()->GetTimerManager().SetTimer(
			EndTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				K2_EndAbility();
			}),
			TotalDuration,
			false
		);
		SpawnTimerHandles.Add(EndTimer);
	}
}

void UPRGA_Leviathan_OrbitalRain::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& TimerHandle : SpawnTimerHandles)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	SpawnTimerHandles.Empty();

	if (IsValid(MontageTask))
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_Leviathan_OrbitalRain::SpawnStrikes()
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !OrbitalStrikeClass)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	// DamageSpec 생성
	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (!DamageSpec.IsValid())
	{
		return;
	}

	// SetByCaller로 데미지 값 설정
	DamageSpec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, BaseDamage);

	const int32 StrikeCount = FMath::RandRange(MinStrikeCount, MaxStrikeCount);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	for (int32 i = 0; i < StrikeCount; ++i)
	{
		FTimerHandle SpawnTimer;
		World->GetTimerManager().SetTimer(
			SpawnTimer,
			FTimerDelegate::CreateWeakLambda(this, [this, DamageSpec, AvatarActor]()
			{
				UWorld* SpawnWorld = GetWorld();
				if (!IsValid(SpawnWorld) || !OrbitalStrikeClass)
				{
					return;
				}

				FVector SpawnLocation = CalculateRandomGroundLocation();

				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = AvatarActor;
				SpawnParams.Instigator = Cast<APawn>(AvatarActor);
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APROrbitalStrike* Strike = SpawnWorld->SpawnActor<APROrbitalStrike>(
					OrbitalStrikeClass,
					FTransform(FRotator::ZeroRotator, SpawnLocation),
					SpawnParams
				);

				if (IsValid(Strike))
				{
					Strike->InitStrike(DamageSpec, TelegraphColor, TelegraphDuration, StrikeRadius);
				}
			}),
			SpawnInterval * i,
			false
		);
		SpawnTimerHandles.Add(SpawnTimer);
	}
}

FVector UPRGA_Leviathan_OrbitalRain::CalculateRandomGroundLocation() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return FVector::ZeroVector;
	}

	// AIController의 CombatTarget 위치를 기준으로 사용
	FVector Origin = AvatarActor->GetActorLocation();
	if (APawn* Pawn = Cast<APawn>(AvatarActor))
	{
		if (APRAIController* AIController = Cast<APRAIController>(Pawn->GetController()))
		{
			if (AActor* CombatTarget = AIController->GetCombatTarget())
			{
				Origin = CombatTarget->GetActorLocation();
			}
		}
	}

	// 랜덤 방향 및 거리 설정
	const float RandomAngle = FMath::FRandRange(0.f, 360.f);
	const float RandomDist = FMath::FRandRange(0.f, SpawnAreaRadius);
	const FVector Offset = FVector(
		FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDist,
		FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDist,
		0.f
	);

	FVector TargetLocation = Origin + Offset;

	// NavMesh 기반 지면 위치 보정
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (IsValid(NavSys))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(TargetLocation, NavLocation, FVector(500.f, 500.f, 500.f)))
		{
			TargetLocation = NavLocation.Location;
		}
	}

	return TargetLocation;
}

void UPRGA_Leviathan_OrbitalRain::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_Leviathan_OrbitalRain::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_Leviathan_OrbitalRain::OnMontageCancelled()
{
	K2_EndAbility();
}
