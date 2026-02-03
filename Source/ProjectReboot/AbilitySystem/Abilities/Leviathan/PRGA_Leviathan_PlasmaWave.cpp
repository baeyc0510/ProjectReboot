// Fill out your copyright notice in the Description page of Project Settings.

#include "PRGA_Leviathan_PlasmaWave.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Combat/PlasmaWave/PRPlasmaWave.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_Leviathan_PlasmaWave::UPRGA_Leviathan_PlasmaWave()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
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

	// 스폰 위치: 꼬리 소켓 또는 액터 위치
	FVector SpawnLocation = AvatarActor->GetActorLocation();
	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (Mesh->DoesSocketExist(TailSocketName))
			{
				SpawnLocation = Mesh->GetSocketLocation(TailSocketName);
			}
		}
	}

	// 타겟 방향 계산 (후방에서 발사하므로 액터의 후방 벡터 사용)
	const FVector ForwardDir = AvatarActor->GetActorForwardVector();
	// 웨이브는 꼬리에서 후방으로 발사 → 액터의 뒤쪽 방향
	const FVector BaseDirection = -ForwardDir;

	// 부채꼴 3방향 스폰
	// 중앙
	SpawnSingleWave(SpawnLocation, BaseDirection, DamageSpec);

	// 좌측 (+SpreadAngle)
	const FVector LeftDirection = BaseDirection.RotateAngleAxis(-SpreadAngle, FVector::UpVector);
	SpawnSingleWave(SpawnLocation, LeftDirection, DamageSpec);

	// 우측 (-SpreadAngle)
	const FVector RightDirection = BaseDirection.RotateAngleAxis(SpreadAngle, FVector::UpVector);
	SpawnSingleWave(SpawnLocation, RightDirection, DamageSpec);
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
