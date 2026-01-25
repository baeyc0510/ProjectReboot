// PRGA_AOEAttack.cpp
#include "PRGA_AOEAttack.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Combat/CombatBlueprintFunctionLibrary.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_AOEAttack::UPRGA_AOEAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPRGA_AOEAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HitActors.Reset();

	// 몽타주가 없으면 즉시 종료
	if (!IsValid(AttackMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 공격 몽타주 재생
	PlayAttackMontage();

	// 공격 이벤트 대기
	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack,
		nullptr,
		!bAllowMultipleEvents,
		true
	);

	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UPRGA_AOEAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (WaitEventTask)
	{
		WaitEventTask->EndTask();
		WaitEventTask = nullptr;
	}
}

void UPRGA_AOEAttack::PlayAttackMontage()
{
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
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

void UPRGA_AOEAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	// 중심 위치 기준으로 범위 트레이스
	PerformAOETrace(&Payload);
}

void UPRGA_AOEAttack::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_AOEAttack::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_AOEAttack::OnMontageCancelled()
{
	K2_EndAbility();
}

void UPRGA_AOEAttack::PerformAOETrace(const FGameplayEventData* TriggerEventData)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return;
	}

	// 트레이스 중심 위치 계산
	const FVector TraceCenter = ResolveTargetLocation(TriggerEventData);
	const FVector TraceStart = TraceCenter;
	const FVector TraceEnd = TraceCenter;
	const FVector TraceDirection = AvatarActor->GetActorForwardVector();

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(AvatarActor);

	// 박스 회전 설정
	FRotator BoxRotation = FRotator::ZeroRotator;
	if (TraceSettings.TraceShape == EPRTraceShape::Box)
	{
		if (TraceSettings.bUseActorRotationForBox)
		{
			BoxRotation = AvatarActor->GetActorRotation() + TraceSettings.BoxRotationOffset;
		}
		else
		{
			BoxRotation = TraceSettings.BoxRotationOffset;
		}
	}

	// 설정 기반 트레이스 수행
	const bool bHit = UCombatBlueprintFunctionLibrary::TraceBySettings(
		AvatarActor,
		TraceStart,
		TraceEnd,
		TraceDirection,
		TraceSettings,
		QueryParams,
		HitResults,
		BoxRotation
	);

	if (!bHit)
	{
		return;
	}

	// 결과 필터링 및 데미지 적용
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActors.Contains(HitActor))
		{
			continue;
		}

		if (TraceSettings.TraceTargetClass && !HitActor->IsA(TraceSettings.TraceTargetClass))
		{
			continue;
		}

		// 중복 처리 방지 등록
		HitActors.Add(HitActor);
		ApplyAOEDamage(HitResult);

		// 최대 타격 수 제한
		if (TraceSettings.MaxHitCount > 0 && HitActors.Num() >= TraceSettings.MaxHitCount)
		{
			break;
		}
	}
}

FVector UPRGA_AOEAttack::ResolveTargetLocation(const FGameplayEventData* TriggerEventData) const
{
	// 소켓 위치를 우선 사용
	if (bUseTraceCenterSocket && !TraceCenterSocketName.IsNone())
	{
		if (USkeletalMeshComponent* MeshComp = GetMeshComponent())
		{
			if (MeshComp->DoesSocketExist(TraceCenterSocketName))
			{
				const FTransform SocketTransform = MeshComp->GetSocketTransform(TraceCenterSocketName, RTS_World);
				return SocketTransform.TransformPosition(TraceCenterSocketOffset);
			}
			UE_LOG(LogTemp, Warning, TEXT("%s: %s doesn't have socket %s"), *GetName(), *MeshComp->GetName(), *TraceCenterSocketName.ToString());
		}
	}

	// 이벤트 타겟 데이터를 우선 사용
	if (bUseEventTargetLocation && TriggerEventData)
	{
		if (TriggerEventData->TargetData.Num() > 0)
		{
			if (const FGameplayAbilityTargetData* TargetData = TriggerEventData->TargetData.Get(0))
			{
				if (TargetData->HasHitResult())
				{
					return TargetData->GetHitResult()->Location + TargetLocationOffset;
				}

				return TargetData->GetEndPoint() + TargetLocationOffset;
			}
		}

		if (IsValid(TriggerEventData->Target))
		{
			return TriggerEventData->Target->GetActorLocation() + TargetLocationOffset;
		}
	}

	// 폴백: 자신 위치
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		return AvatarActor->GetActorLocation() + TargetLocationOffset;
	}

	return TargetLocationOffset;
}

USkeletalMeshComponent* UPRGA_AOEAttack::GetMeshComponent() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		return Character->GetMesh();
	}

	return AvatarActor->FindComponentByClass<USkeletalMeshComponent>();
}

void UPRGA_AOEAttack::ApplyAOEDamage(const FHitResult& HitResult)
{
	if (!IsValid(DamageEffectClass))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!IsValid(TargetASC))
	{
		return;
	}

	// 타겟에 데미지 GE 적용
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, BaseDamage);

		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetDataHandle.Add(TargetData);

		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, TargetDataHandle);
	}
}
