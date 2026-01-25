// PRGA_MeleeAttack.cpp
#include "PRGA_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/CombatBlueprintFunctionLibrary.h"

UPRGA_MeleeAttack::UPRGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 기본 트레이스 설정
	TraceSettings.TraceShape = EPRTraceShape::Sphere;
	TraceSettings.TraceRadius = 25.0f;
	TraceSettings.TraceChannel = ECC_Pawn;
	TraceSettings.MaxHitCount = 1;
	TraceSettings.bDrawDebugTrace = false;
	TraceSettings.DebugDrawTime = 1.0f;
}

void UPRGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 히트 캐시 초기화
	HitActors.Reset();

	if (!IsValid(AttackMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 공격 몽타주 재생
	PlayAttackMontage();

	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack,
		nullptr,
		!bAllowMultipleEvents,
		true
	);

	// 공격 이벤트 수신
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UPRGA_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

void UPRGA_MeleeAttack::PlayAttackMontage()
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

void UPRGA_MeleeAttack::OnAttackEventReceived(FGameplayEventData Payload)
{
	PerformAttackTrace();
}

void UPRGA_MeleeAttack::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_MeleeAttack::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_MeleeAttack::OnMontageCancelled()
{
	K2_EndAbility();
}

void UPRGA_MeleeAttack::PerformAttackTrace()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return;
	}
	
	USkeletalMeshComponent* MeshComp = GetMeshComponent();
	if (!IsValid(MeshComp))
	{
		return;
	}

	TArray<FHitResult> HitResults;
	
	// Trace 설정
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(AvatarActor);

	// 소켓 유효성 체크
	if (!TraceStartSocketName.IsNone() && !MeshComp->DoesSocketExist(TraceStartSocketName))
	{
		UE_LOG(LogTemp,Warning,TEXT("%s: %s doesn't have socket %s"),*GetName(), *AvatarActor->GetName(),*TraceStartSocketName.ToString());
		return;
	}

	FVector TraceStart;
	FVector TraceDirection;
	if (!TraceStartSocketName.IsNone())
	{
		const FTransform StartSocketTransform = MeshComp->GetSocketTransform(TraceStartSocketName, RTS_World);
		TraceStart = StartSocketTransform.TransformPosition(TraceStartSocketOffset);
		TraceDirection = (TraceDirectionType == EPRMeleeTraceDirection::ActorForward) ? AvatarActor->GetActorForwardVector() : StartSocketTransform.GetRotation().GetForwardVector();
	}
	else
	{
		TraceStart = AvatarActor->GetActorLocation();
		TraceDirection = AvatarActor->GetActorForwardVector();
	}
	
	// 트레이스 구간 계산
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;
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
		MeshComp,
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
		ApplyMeleeDamage(HitResult);
		
		// 최대 타격 수 제한
		if (TraceSettings.MaxHitCount > 0 && HitActors.Num() >= TraceSettings.MaxHitCount)
		{
			break;
		}
	}
}

void UPRGA_MeleeAttack::ApplyMeleeDamage(const FHitResult& HitResult)
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

USkeletalMeshComponent* UPRGA_MeleeAttack::GetMeshComponent() const
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
