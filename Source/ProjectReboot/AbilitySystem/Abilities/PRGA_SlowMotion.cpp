// PRGA_SlowMotion.cpp
#include "PRGA_SlowMotion.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_SlowMotion::UPRGA_SlowMotion()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 사망 중에는 슬로우 모션 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);

	// 어빌리티 태그 설정
	AbilityTags.AddTag(TAG_Ability_SlowMotion);

	// 슬로우 모션 중 재발동 방지
	ActivationBlockedTags.AddTag(TAG_Ability_SlowMotion);
}

void UPRGA_SlowMotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ApplySlowMotion();

	// WaitDelay는 게임 시간 기준이므로, 실제 시간으로 변환
	// 실제 시간 2초 = 게임 시간 2초 * GlobalTimeDilation
	const float AdjustedDelay = SlowMotionDuration * GlobalTimeDilation;

	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, AdjustedDelay);
	WaitDelayTask->OnFinish.AddDynamic(this, &ThisClass::OnSlowMotionDurationExpired);
	WaitDelayTask->ReadyForActivation();
}

void UPRGA_SlowMotion::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 시간 복원
	RestoreNormalTime();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_SlowMotion::ApplySlowMotion()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	// 원래 값 저장
	OriginalGlobalTimeDilation = World->GetWorldSettings()->TimeDilation;
	if (IsValid(AvatarActor))
	{
		OriginalPlayerTimeDilation = AvatarActor->CustomTimeDilation;
	}

	// 글로벌 타임 딜레이션 적용 (월드 슬로우)
	UGameplayStatics::SetGlobalTimeDilation(World, GlobalTimeDilation);

	// 플레이어 타임 딜레이션 보정 (정상 속도 유지)
	if (IsValid(AvatarActor) && GlobalTimeDilation > 0.0f)
	{
		// 글로벌 딜레이션을 상쇄하여 정상 속도 유지
		float CompensatedDilation = PlayerTimeDilationMultiplier / GlobalTimeDilation;
		AvatarActor->CustomTimeDilation = CompensatedDilation;
	}
}

void UPRGA_SlowMotion::RestoreNormalTime()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 글로벌 타임 딜레이션 복원
	UGameplayStatics::SetGlobalTimeDilation(World, OriginalGlobalTimeDilation);

	// 플레이어 타임 딜레이션 복원
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (IsValid(AvatarActor))
	{
		AvatarActor->CustomTimeDilation = OriginalPlayerTimeDilation;
	}
}

void UPRGA_SlowMotion::OnSlowMotionDurationExpired()
{
	K2_EndAbility();
}
