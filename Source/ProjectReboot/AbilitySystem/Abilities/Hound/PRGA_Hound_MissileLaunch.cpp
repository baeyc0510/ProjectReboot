// PRGA_Hound_MissileLaunch.cpp

#include "PRGA_Hound_MissileLaunch.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "NavigationSystem.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/HoundAOE/PRHoundAOEZone.h"

UPRGA_Hound_MissileLaunch::UPRGA_Hound_MissileLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPRGA_Hound_MissileLaunch::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	Super::GetPrewarmChildren(OutChildren);
	if (IsValid(AOEZoneClass))
	{
		OutChildren.Add(AOEZoneClass);
	}
}

void UPRGA_Hound_MissileLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 결정
	if (bUseMontageOverride)
	{
		MontageToPlay = CastMontage;
	}
	else
	{
		MontageToPlay = FindMontageByGameplayTag(CastMontageTag);
	}

	if (!IsValid(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 몽타주 재생
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		MontagePlayRate,
		NAME_None,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	// Attack 이벤트 대기
	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Attack,
		nullptr,
		false,
		true
	);

	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UPRGA_Hound_MissileLaunch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

void UPRGA_Hound_MissileLaunch::OnAttackEventReceived(FGameplayEventData Payload)
{
	SpawnAOEZone();
}

void UPRGA_Hound_MissileLaunch::SpawnAOEZone()
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !AOEZoneClass)
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

	// 데미지/지속시간/간격 설정
	DamageSpec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, BaseDamage);
	DamageSpec.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Duration, DamageDuration);
	DamageSpec.Data->Period = DamagePeriod;

	// 타겟 위치 계산
	FVector SpawnLocation = CalculateTargetLocation();

	// AOE Zone 스폰
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APRHoundAOEZone* Zone = World->SpawnActor<APRHoundAOEZone>(
		AOEZoneClass,
		FTransform(FRotator::ZeroRotator, SpawnLocation),
		SpawnParams
	);

	if (IsValid(Zone))
	{
		Zone->InitZone(DamageSpec, TelegraphColor, TelegraphDuration, StrikeRadius, DamageDuration, TargetActorClass);
	}
}

FVector UPRGA_Hound_MissileLaunch::CalculateTargetLocation() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return FVector::ZeroVector;
	}

	// AIController의 CombatTarget 위치 사용
	FVector TargetLocation = AvatarActor->GetActorLocation();
	if (APawn* Pawn = Cast<APawn>(AvatarActor))
	{
		if (APRAIController* AIController = Cast<APRAIController>(Pawn->GetController()))
		{
			if (AActor* CombatTarget = AIController->GetCombatTarget())
			{
				TargetLocation = CombatTarget->GetActorLocation();
			}
		}
	}

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

void UPRGA_Hound_MissileLaunch::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_Hound_MissileLaunch::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_Hound_MissileLaunch::OnMontageCancelled()
{
	K2_EndAbility();
}
