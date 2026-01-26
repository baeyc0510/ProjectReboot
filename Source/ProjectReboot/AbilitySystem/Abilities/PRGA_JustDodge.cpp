// PRGA_JustDodge.cpp
#include "PRGA_JustDodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/JustDodge/PRJustDodgeGhost.h"

UPRGA_JustDodge::UPRGA_JustDodge()
{
	// 사망 중에는 저스트 회피 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);

	// 어빌리티 태그 설정
	AbilityTags.AddTag(TAG_Ability_JustDodge);

	// Event.Dash 이벤트로 트리거
	FAbilityTriggerData TriggerData;
	{
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		TriggerData.TriggerTag = TAG_Event_Dash;
	}
	AbilityTriggers.Add(TriggerData);
}

void UPRGA_JustDodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bJustDodgeSucceeded = false;
	
	if (!IsValidChecked(DodgingStateEffectClass))
	{
		UE_LOG(LogTemp,Warning,TEXT("UPRGA_JustDodge: DodgingStateEffectClas not set"));
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	// 회피 상태 GE 적용 (State.Dodging 태그 부여)
	ApplyDodgingState();

	// 저스트 회피 잔상 소환
	SpawnJustDodgeGhost();

	// 데미지 이벤트 대기 시작
	StartWaitingForDamageEvent();

	// 잔상 히트 이벤트 대기 시작
	StartWaitingForHitGhostEvent();
}

void UPRGA_JustDodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// Task 정리
	if (WaitDamageEventTask)
	{
		WaitDamageEventTask->EndTask();
		WaitDamageEventTask = nullptr;
	}

	if (WaitHitGhostEventTask)
	{
		WaitHitGhostEventTask->EndTask();
		WaitHitGhostEventTask = nullptr;
	}

	if (WaitDodgingStateRemovedTask)
	{
		WaitDodgingStateRemovedTask->EndTask();
		WaitDodgingStateRemovedTask = nullptr;
	}

	// 회피 상태 GE 제거 (아직 유효한 경우)
	if (DodgingEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(DodgingEffectHandle);
		}
		DodgingEffectHandle.Invalidate();
	}

	if (IsValid(JustDodgeGhost))
	{
		JustDodgeGhost->Destroy();
		JustDodgeGhost = nullptr;
	}
}

void UPRGA_JustDodge::ApplyDodgingState()
{
	if (!IsValid(DodgingStateEffectClass))
	{
		UE_LOG(LogTemp,Warning,TEXT("UPRGA_JustDodge: DodgingStateEffectClas not set"));
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DodgingStateEffectClass, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		return;
	}

	DodgingEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);

	// 회피 상태 GE 만료 대기
	if (DodgingEffectHandle.IsValid())
	{
		WaitDodgingStateRemovedTask = UAbilityTask_WaitGameplayEffectRemoved::WaitForGameplayEffectRemoved(
			this,
			DodgingEffectHandle
		);
		WaitDodgingStateRemovedTask->OnRemoved.AddDynamic(this, &ThisClass::OnDodgingStateExpired);
		WaitDodgingStateRemovedTask->ReadyForActivation();
	}
}

void UPRGA_JustDodge::StartWaitingForDamageEvent()
{
	WaitDamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Damage,
		nullptr,  // OptionalExternalTarget
		false,    // OnlyTriggerOnce
		false     // OnlyMatchExact
	);
	WaitDamageEventTask->EventReceived.AddDynamic(this, &ThisClass::OnDamageEventReceived);
	WaitDamageEventTask->ReadyForActivation();
}

void UPRGA_JustDodge::StartWaitingForHitGhostEvent()
{
	WaitHitGhostEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_HitGhost,
		nullptr,  // OptionalExternalTarget
		false,    // OnlyTriggerOnce
		false     // OnlyMatchExact
	);
	WaitHitGhostEventTask->EventReceived.AddDynamic(this, &ThisClass::OnHitGhostEventReceived);
	WaitHitGhostEventTask->ReadyForActivation();
}

void UPRGA_JustDodge::SpawnJustDodgeGhost()
{
	if (!IsValid(JustDodgeGhostClass))
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!IsValid(Character))
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	JustDodgeGhost = GetWorld()->SpawnActor<APRJustDodgeGhost>(
		JustDodgeGhostClass,
		Character->GetActorTransform(),
		SpawnParams
	);

	if (!IsValid(JustDodgeGhost))
	{
		return;
	}

	JustDodgeGhost->SetOwnerASC(ASC);
	JustDodgeGhost->InitCapsuleSize(Character->GetCapsuleComponent());
}

void UPRGA_JustDodge::OnDamageEventReceived(FGameplayEventData Payload)
{
	// 이미 성공했으면 무시
	if (bJustDodgeSucceeded)
	{
		return;
	}

	// Dodgeable 태그 확인 (InstigatorTags에 포함됨)
	if (!Payload.InstigatorTags.HasTag(TAG_Damage_Dodgeable))
	{
		return;
	}

	// 저스트 회피 성공
	HandleJustDodgeSuccess(&Payload);
}

void UPRGA_JustDodge::OnHitGhostEventReceived(FGameplayEventData Payload)
{
	// 이미 성공했으면 무시
	if (bJustDodgeSucceeded)
	{
		return;
	}

	// 저스트 회피 성공
	HandleJustDodgeSuccess(&Payload);
}

void UPRGA_JustDodge::HandleJustDodgeSuccess(const FGameplayEventData* DamageEventData)
{
	bJustDodgeSucceeded = true;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		K2_EndAbility();
		return;
	}

	// 저스트 회피 성공 이벤트 발송
	SendJustDodgeEvent(DamageEventData);

	// 보상 GE 적용
	if (IsValid(JustDodgeRewardEffectClass))
	{
		FGameplayEffectSpecHandle RewardSpecHandle = MakeOutgoingGameplayEffectSpec(JustDodgeRewardEffectClass, GetAbilityLevel());
		if (RewardSpecHandle.IsValid())
		{
			// 원본 데미지 이벤트의 Context 유지
			if (DamageEventData && DamageEventData->ContextHandle.IsValid())
			{
				RewardSpecHandle.Data->SetContext(DamageEventData->ContextHandle);
			}
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, RewardSpecHandle);
		}
	}

	// 회피 상태 GE 즉시 제거 (성공 후 더 이상 필요 없음)
	if (DodgingEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(DodgingEffectHandle);
		DodgingEffectHandle.Invalidate();
	}

	K2_EndAbility();
}

void UPRGA_JustDodge::SendJustDodgeEvent(const FGameplayEventData* DamageEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_JustDodge;
	EventData.Instigator = GetAvatarActorFromActorInfo();
	EventData.Target = GetAvatarActorFromActorInfo();

	// 원본 데미지 이벤트의 정보 전달
	if (DamageEventData)
	{
		EventData.EventMagnitude = DamageEventData->EventMagnitude;
		EventData.ContextHandle = DamageEventData->ContextHandle;
	}

	ASC->HandleGameplayEvent(TAG_Event_JustDodge, &EventData);
}

void UPRGA_JustDodge::OnDodgingStateExpired(const FGameplayEffectRemovalInfo& RemovalInfo)
{
	// 회피 윈도우 만료 - 저스트 회피 실패
	DodgingEffectHandle.Invalidate();

	// 이미 성공했으면 무시 (성공 시에도 GE 제거되어 콜백 호출될 수 있음)
	if (bJustDodgeSucceeded)
	{
		return;
	}

	K2_EndAbility();
}
