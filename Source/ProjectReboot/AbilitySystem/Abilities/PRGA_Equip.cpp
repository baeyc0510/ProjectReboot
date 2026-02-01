// PRGA_Equip.cpp
#include "PRGA_Equip.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "TimerManager.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Animation/PRAnimationBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"

UPRGA_Equip::UPRGA_Equip()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
	
	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active);
}

void UPRGA_Equip::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UPRGA_Equip::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (ShowEquipmentWaitTask)
	{
		ShowEquipmentWaitTask->EndTask();
		ShowEquipmentWaitTask = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideEquipmentTimerHandle);
	}

	ShowHiddenEquipment();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UPRGA_Equip::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !TriggerEventData)
	{
		return;
	}

	// 장착 이벤트 데이터 기반으로 장비를 다음 프레임에 숨김
	const UPREquipActionData* EquipAction = Cast<UPREquipActionData>(TriggerEventData->OptionalObject);
	ScheduleHideEquipment(EquipAction);

	FGameplayTag MontageTag;
	if (!TryGetMontageTagForEventTag(TriggerEventData->EventTag, MontageTag))
	{
		// 재생할 몽타주가 없으면 숨김 복구 후 종료
		ShowHiddenEquipment();
		K2_EndAbility();
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		// 아바타가 유효하지 않으면 숨김 복구 후 종료
		ShowHiddenEquipment();
		K2_EndAbility();
		return;
	}

	UAnimMontage* MontageToPlay = UPRAnimationBlueprintLibrary::FindAnimMontageByGameplayTag(AvatarActor, MontageTag);
	if (!IsValid(MontageToPlay))
	{
		// 몽타주가 유효하지 않으면 숨김 복구 후 종료
		ShowHiddenEquipment();
		K2_EndAbility();
		return;
	}

	PlayEquipMontage(MontageToPlay);
}

void UPRGA_Equip::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (ShowEquipmentWaitTask)
	{
		ShowEquipmentWaitTask->EndTask();
		ShowEquipmentWaitTask = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideEquipmentTimerHandle);
	}

	ShowHiddenEquipment();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_Equip::PlayEquipMontage(UAnimMontage* MontageToPlay)
{
	if (!IsValid(MontageToPlay))
	{
		return;
	}

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		MontagePlayRate,
		NAME_None,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::HandleMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageFinished);

	MontageTask->ReadyForActivation();
}

bool UPRGA_Equip::TryGetMontageTagForEventTag(const FGameplayTag& EventTag, FGameplayTag& OutMontageTag) const
{
	int32 BestDepth = -1;
	FGameplayTag BestTag;

	for (const TPair<FGameplayTag, FGameplayTag>& Pair : EventTagToMontageTag)
	{
		if (!Pair.Key.IsValid() || !Pair.Value.IsValid())
		{
			continue;
		}

		if (!EventTag.MatchesTag(Pair.Key))
		{
			continue;
		}

		const int32 Depth = Pair.Key.GetGameplayTagParents().Num();
		if (Depth > BestDepth)
		{
			BestDepth = Depth;
			BestTag = Pair.Value;
		}
	}

	if (!BestTag.IsValid())
	{
		return false;
	}

	OutMontageTag = BestTag;
	return true;
}

void UPRGA_Equip::HandleMontageFinished()
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	// 몽타주 종료 시 장비 노출 복구
	ShowHiddenEquipment();

	K2_EndAbility();
}

void UPRGA_Equip::ScheduleHideEquipment(const UPREquipActionData* EquipAction)
{
	if (!IsValid(EquipAction))
	{
		return;
	}

	// 장비 인스턴스 생성 지연을 고려해 다음 프레임에 숨김 처리
	PendingEquipAction = EquipAction;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideEquipmentTimerHandle);
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleDeferredHideEquipment);
	}
	else
	{
		TryHideEquipment(EquipAction);
	}
}

void UPRGA_Equip::TryHideEquipment(const UPREquipActionData* EquipAction)
{
	if (!IsValid(EquipAction) || !EquipAction->EquipmentSlot.IsValid())
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return;
	}

	UPREquipmentManagerComponent* EquipmentManager = UPREquipmentBlueprintLibrary::GetEquipmentManager(AvatarActor);
	if (!IsValid(EquipmentManager))
	{
		return;
	}

	// 슬롯 장비 비주얼 숨김 처리
	HiddenEquipmentSlot = EquipAction->EquipmentSlot;
	CachedEquipmentManager = EquipmentManager;
	EquipmentManager->SetEquipmentVisibility(HiddenEquipmentSlot, false);
	bIsEquipmentHidden = true;

	if (ShowEquipmentWaitTask)
	{
		ShowEquipmentWaitTask->EndTask();
		ShowEquipmentWaitTask = nullptr;
	}

	ShowEquipmentWaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_ShowEquipment,
		nullptr,
		false,
		true
	);

	// 장비 노출 이벤트 수신 대기
	ShowEquipmentWaitTask->EventReceived.AddDynamic(this, &ThisClass::OnShowEquipmentEventReceived);
	ShowEquipmentWaitTask->ReadyForActivation();
}

void UPRGA_Equip::HandleDeferredHideEquipment()
{
	// 다음 프레임에 지연된 장비 숨김 처리
	TryHideEquipment(PendingEquipAction.Get());
}

void UPRGA_Equip::ShowHiddenEquipment()
{
	if (!bIsEquipmentHidden)
	{
		return;
	}

	// 숨긴 슬롯 장비 비주얼 복구
	if (UPREquipmentManagerComponent* EquipmentManager = CachedEquipmentManager.Get())
	{
		EquipmentManager->SetEquipmentVisibility(HiddenEquipmentSlot, true);
	}

	bIsEquipmentHidden = false;
	HiddenEquipmentSlot = FGameplayTag();
	CachedEquipmentManager.Reset();

	if (ShowEquipmentWaitTask)
	{
		ShowEquipmentWaitTask->EndTask();
		ShowEquipmentWaitTask = nullptr;
	}
}

void UPRGA_Equip::OnShowEquipmentEventReceived(FGameplayEventData Payload)
{
	// 노출 이벤트 수신 시 장비 복구
	ShowHiddenEquipment();
}
