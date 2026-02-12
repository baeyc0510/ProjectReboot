// Fill out your copyright notice in the Description page of Project Settings.

#include "PRGA_Leviathan_PlasmaWave.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjectReboot/Combat/PlasmaWave/PRPlasmaWave.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_Leviathan_PlasmaWave::UPRGA_Leviathan_PlasmaWave()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPRGA_Leviathan_PlasmaWave::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	Super::GetPrewarmChildren(OutChildren);
	if (IsValid(PlasmaWaveClass))
	{
		OutChildren.Add(PlasmaWaveClass);
	}
}

void UPRGA_Leviathan_PlasmaWave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 재생
	if (!IsValid(BackflipMontage))
	{
		// 몽타주 없으면 즉시 웨이브 발사 후 종료
		SpawnWaves();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		BackflipMontage,
		MontagePlayRate,
		NAME_None,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 공격 이벤트 대기 (AnimNotify에서 Event.Attack 발송)
	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack,
		nullptr,
		true,
		true
	);

	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UPRGA_Leviathan_PlasmaWave::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsValid(MontageTask))
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (IsValid(WaitEventTask))
	{
		WaitEventTask->EndTask();
		WaitEventTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_Leviathan_PlasmaWave::OnAttackEventReceived(FGameplayEventData Payload)
{
	SpawnWaves();
}

void UPRGA_Leviathan_PlasmaWave::SpawnWaves()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor) || !PlasmaWaveClass)
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

	// 스폰 위치: 액터 아래 지면
	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (UWorld* World = GetWorld())
	{
		const float GroundTraceHeight = 3000.f;
		const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, GroundTraceHeight);
		const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, GroundTraceHeight);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(AvatarActor);

		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			SpawnLocation.Z = HitResult.Location.Z;
		}
	}

	// 타겟 방향 계산
	FVector Direction = AvatarActor->GetActorForwardVector() * FVector(1.0f,1.0f,0.0f);
	Direction.Normalize();

	// 부채꼴 3방향 스폰
	const FVector RightVector = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal();

	// 중앙
	SpawnSingleWave(SpawnLocation, Direction, DamageSpec);

	// 좌측 (+SpreadAngle, 좌로 오프셋)
	const FVector LeftDirection = Direction.RotateAngleAxis(-SpreadAngle, FVector::UpVector);
	const FVector LeftSpawnLocation = SpawnLocation - RightVector * SpawnLateralSpacing;
	SpawnSingleWave(LeftSpawnLocation, LeftDirection, DamageSpec);

	// 우측 (-SpreadAngle, 우로 오프셋)
	const FVector RightDirection = Direction.RotateAngleAxis(SpreadAngle, FVector::UpVector);
	const FVector RightSpawnLocation = SpawnLocation + RightVector * SpawnLateralSpacing;
	SpawnSingleWave(RightSpawnLocation, RightDirection, DamageSpec);
}

void UPRGA_Leviathan_PlasmaWave::SpawnSingleWave(const FVector& SpawnLocation, const FVector& Direction, const FGameplayEffectSpecHandle& InDamageSpec)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APRPlasmaWave* Wave = World->SpawnActor<APRPlasmaWave>(
		PlasmaWaveClass,
		FTransform(Direction.Rotation(), SpawnLocation),
		SpawnParams
	);

	if (IsValid(Wave))
	{
		Wave->InitWave(Direction, InDamageSpec);
	}
}

void UPRGA_Leviathan_PlasmaWave::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_Leviathan_PlasmaWave::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_Leviathan_PlasmaWave::OnMontageCancelled()
{
	K2_EndAbility();
}
