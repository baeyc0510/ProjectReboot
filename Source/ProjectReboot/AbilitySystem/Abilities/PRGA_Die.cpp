// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGA_Die.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"
#include "ProjectReboot/Combat/CombatBlueprintFunctionLibrary.h"
#include "ProjectReboot/Combat/CombatTypes.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

UPRGA_Die::UPRGA_Die()
{
	AbilityTags.AddTag(TAG_Ability_Die);
	
	// 이미 Dead 상태인 경우 중복 발동 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);
	
	// 다른 어빌리티 발동 취소 및 차단
	CancelAbilitiesWithTag.AddTag(TAG_Ability_Active);
	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active);
	
	// GameplayEvent를 통한 발동 조건 추가
	FAbilityTriggerData TriggerData;
	{
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		TriggerData.TriggerTag = TAG_Event_Death;	
	}
	AbilityTriggers.Add(TriggerData);
}

void UPRGA_Die::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 1. 사망 상태 GE 적용
	ApplyDeathState(TriggerEventData);

	// 2. 몽타주 재생
	if  (UAnimMontage* DeathMontage = GetDeathMontage(TriggerEventData))
	{
		PlayDeathMontage(DeathMontage);
	}
}

void UPRGA_Die::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
}

void UPRGA_Die::ApplyDeathState(const FGameplayEventData* TriggerEventData)
{
	if (!IsValid(DeathStateEffectClass))
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DeathStateEffectClass, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		return;
	}

	// 원본 Context 유지
	if (TriggerEventData && TriggerEventData->ContextHandle.IsValid())
	{
		SpecHandle.Data->SetContext(TriggerEventData->ContextHandle);
	}

	ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
}

UAnimMontage* UPRGA_Die::GetDeathMontage(const FGameplayEventData* TriggerEventData) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	// 오버라이드 몽타주 우선
	if (IsValid(DeathMontageOverride))
	{
		return DeathMontageOverride;
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
	FGameplayTag DirectionalTag = GetDeathMontageTagByDirection(HitDirection);
	if (UAnimMontage* DirectionalMontage = ARI->FindMontageByGameplayTag(DirectionalTag))
	{
		return DirectionalMontage;
	}

	// Fallback: FrontDeathMontage 태그
	if (UAnimMontage* FrontDeathMontage = ARI->FindMontageByGameplayTag(TAG_Montage_Death_Front))
	{
		return FrontDeathMontage;
	}
	
	// Fallback: 기본 DeathMontage 태그
	return ARI->FindMontageByGameplayTag(TAG_Montage_Death);
}

FGameplayTag UPRGA_Die::GetDeathMontageTagByDirection(EPRHitDirection HitDirection) const
{
	switch (HitDirection)
	{
	case EPRHitDirection::Front:
		return TAG_Montage_Death_Front;
	case EPRHitDirection::Right:
		return TAG_Montage_Death_Right;
	case EPRHitDirection::Back:
		return TAG_Montage_Death_Back;
	case EPRHitDirection::Left:
		return TAG_Montage_Death_Left;
	default:
		return TAG_Montage_Death;
	}
}

void UPRGA_Die::PlayDeathMontage(UAnimMontage* MontageToPlay)
{
	if (IsValid(MontageToPlay))
	{
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
}

void UPRGA_Die::OnMontageCompleted()
{
	K2_OnDeathFinished();
}


void UPRGA_Die::OnMontageBlendOut()
{
	K2_OnDeathFinished();
}

void UPRGA_Die::OnMontageCancelled()
{
	K2_OnDeathFinished();
}

void UPRGA_Die::FinishDie()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		// 소유 액터 없음
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(AvatarActor))
	{
		// 전투 인터페이스를 통한 마무리
		CombatInterface->FinishDie();
	}
}

void UPRGA_Die::K2_OnDeathFinished_Implementation()
{
	FinishDie();
}
