// PRGA_MeleeAttack.cpp
#include "PRGA_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/CombatBlueprintFunctionLibrary.h"

UPRGA_MeleeAttack::UPRGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
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

	HitActors.Reset();

	if (!IsValid(AttackMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	PlayAttackMontage();

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
	
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;
	
	// Trace 시작
	const bool bHit = UCombatBlueprintFunctionLibrary::SphereSweepTraceByStartEnd(
		MeshComp,
		TraceStart,
		TraceEnd,
		TraceDirection,
		TraceRadius,
		TraceChannel,
		QueryParams,
		HitResults,
		bDrawDebugTrace,
		DebugDrawTime
	);

	if (!bHit)
	{
		return;
	}

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
		
		if (TraceTargetClass && !HitActor->IsA(TraceTargetClass))
		{
			continue;
		}

		HitActors.Add(HitActor);
		ApplyMeleeDamage(HitResult);
		
		if (HitActors.Num() == MaxTraceCount)
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
