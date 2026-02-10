// PRGA_SlowMotion.cpp
#include "PRGA_SlowMotion.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
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

	// GCN 활성화 (슬로우 모션 연출 시작, 어빌리티 종료 시 자동 제거)
	K2_AddGameplayCue(TAG_GameplayCue_Character_SlowMotion, FGameplayEffectContextHandle(), true);

	// WaitDelay는 게임 시간 기준이므로, 실제 시간으로 변환
	const float AdjustedDelay = SlowMotionDuration * GlobalTimeDilation;

	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, AdjustedDelay);
	WaitDelayTask->OnFinish.AddDynamic(this, &ThisClass::OnSlowMotionDurationExpired);
	WaitDelayTask->ReadyForActivation();
}

void UPRGA_SlowMotion::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// K2_AddGameplayCue의 bRemoveOnAbilityEnd=true로 자동 제거됨
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_SlowMotion::OnSlowMotionDurationExpired()
{
	K2_EndAbility();
}
