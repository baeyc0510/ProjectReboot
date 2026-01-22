// PRGA_Hit.cpp
#include "PRGA_Hit.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"
#include "ProjectReboot/Combat/CombatBlueprintFunctionLibrary.h"
#include "ProjectReboot/Combat/CombatTypes.h"

UPRGA_Hit::UPRGA_Hit()
{
	// 사망 중에는 피격 재생 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);

	// GameplayEvent 기반 발동
	FAbilityTriggerData TriggerData;
	{
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		TriggerData.TriggerTag = TAG_Event_Hit;
	}
	AbilityTriggers.Add(TriggerData);
}

void UPRGA_Hit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAnimMontage* HitMontage = GetHitMontage(TriggerEventData))
	{
		PlayHitMontage(HitMontage);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPRGA_Hit::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
}

void UPRGA_Hit::PlayHitMontage(UAnimMontage* MontageToPlay)
{
	if (!IsValid(MontageToPlay))
	{
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		MontagePlayRate,
		NAME_None,
		false  // bStopWhenAbilityEnds
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

UAnimMontage* UPRGA_Hit::GetHitMontage(const FGameplayEventData* TriggerEventData) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	// 오버라이드 몽타주 우선
	if (IsValid(HitMontageOverride))
	{
		return HitMontageOverride;
	}

	IPRAnimRegistryInterface* ARI = Cast<IPRAnimRegistryInterface>(AvatarActor);
	if (!ARI)
	{
		return nullptr;
	}

	// 피격 방향 계산
	EPRHitDirection HitDirection = EPRHitDirection::Front;
	if (TriggerEventData && TriggerEventData->ContextHandle.IsValid())
	{
		if (const FHitResult* HitResult = TriggerEventData->ContextHandle.GetHitResult())
		{
			HitDirection = UCombatBlueprintFunctionLibrary::GetHitDirectionFromHitResult(*HitResult, AvatarActor);
		}
		else if (IsValid(TriggerEventData->Instigator))
		{
			HitDirection = UCombatBlueprintFunctionLibrary::GetHitDirectionFromInstigator(TriggerEventData->Instigator, AvatarActor);
		}
	}

	// 방향별 태그로 몽타주 검색
	FGameplayTag DirectionalTag = GetHitMontageTagByDirection(HitDirection);
	if (UAnimMontage* DirectionalMontage = ARI->FindMontageByGameplayTag(DirectionalTag))
	{
		return DirectionalMontage;
	}

	// Fallback: BackHitMontage 태그
	if (UAnimMontage* FrontHitMontage = ARI->FindMontageByGameplayTag(TAG_Montage_Hit_Back))
	{
		return FrontHitMontage;
	}

	// Fallback: 기본 HitMontage 태그
	return ARI->FindMontageByGameplayTag(TAG_Montage_Hit);
}

FGameplayTag UPRGA_Hit::GetHitMontageTagByDirection(EPRHitDirection HitDirection) const
{
	switch (HitDirection)
	{
	case EPRHitDirection::Front:
		return TAG_Montage_Hit_Front;
	case EPRHitDirection::Right:
		return TAG_Montage_Hit_Right;
	case EPRHitDirection::Back:
		return TAG_Montage_Hit_Back;
	case EPRHitDirection::Left:
		return TAG_Montage_Hit_Left;
	default:
		return TAG_Montage_Hit;
	}
}

void UPRGA_Hit::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_Hit::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_Hit::OnMontageCancelled()
{
	K2_EndAbility();
}
