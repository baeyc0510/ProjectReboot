// PRGA_Dash.cpp
#include "PRGA_Dash.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"

UPRGA_Dash::UPRGA_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 사망 중에는 대시 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);

	// 대시 중에는 재발동 방지
	ActivationBlockedTags.AddTag(TAG_Ability_Dash);

	// 어빌리티 태그 설정
	AbilityTags.AddTag(TAG_Ability_Dash);

	// 입력 기반 발동
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
}

void UPRGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 대시 이벤트 발송 (저스트 회피 트리거)
	SendDashEvent();

	// 대시 이동 적용
	ApplyDashMovement();

	// 대시 몽타주 재생
	if (UAnimMontage* DashMontage = GetDashMontage())
	{
		PlayDashMontage();
		return;
	}

	// 몽타주 없으면 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPRGA_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (DashMoveTask)
	{
		DashMoveTask->EndTask();
		DashMoveTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_Dash::SendDashEvent()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_Dash;
	EventData.Instigator = GetAvatarActorFromActorInfo();
	EventData.Target = GetAvatarActorFromActorInfo();

	ASC->HandleGameplayEvent(TAG_Event_Dash, &EventData);
}

void UPRGA_Dash::PlayDashMontage()
{
	UAnimMontage* MontageToPlay = GetDashMontage();
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

FVector UPRGA_Dash::GetDashDirection() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!IsValid(Character))
	{
		return FVector::ForwardVector;
	}

	// 월드 공간의 이동 입력
	FVector WorldInputDir = Character->GetLastMovementInputVector();
	if (WorldInputDir.IsNearlyZero())
	{
		// 입력 없으면 앞으로
		return FVector::ForwardVector;
	}

	// 캐릭터 로컬 공간으로 변환
	const FRotator CharacterRotation = Character->GetActorRotation();
	const FVector LocalDir = CharacterRotation.UnrotateVector(WorldInputDir);

	return LocalDir.GetSafeNormal2D();
}

FVector UPRGA_Dash::GetDashWorldDirection() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!IsValid(Character))
	{
		return FVector::ForwardVector;
	}

	// 월드 공간의 이동 입력
	FVector WorldInputDir = Character->GetLastMovementInputVector();
	if (WorldInputDir.IsNearlyZero())
	{
		// 입력 없으면 캐릭터 전방
		return Character->GetActorForwardVector();
	}

	return WorldInputDir.GetSafeNormal2D();
}

FGameplayTag UPRGA_Dash::GetDashMontageTag(const FVector& LocalDirection) const
{
	// X = Forward, Y = Right
	const FVector2D LocalDir2D(LocalDirection.X, LocalDirection.Y);
	const FVector2D NormalizedDir = LocalDir2D.GetSafeNormal();
	const float ForwardDot = NormalizedDir.X;
	const float RightDot = NormalizedDir.Y;

	// 전/후방 60도(±30도), 좌/우측 120도(±60도)
	const float ForwardBackThreshold = FMath::Cos(FMath::DegreesToRadians(30.0f));

	if (ForwardDot >= ForwardBackThreshold)
	{
		return TAG_Montage_Dash_Front;
	}

	if (ForwardDot <= -ForwardBackThreshold)
	{
		return TAG_Montage_Dash_Back;
	}

	return (RightDot >= 0.0f) ? TAG_Montage_Dash_Right : TAG_Montage_Dash_Left;
}

UAnimMontage* UPRGA_Dash::GetDashMontage() const
{
	// 오버라이드 몽타주 우선
	if (IsValid(DashMontageOverride))
	{
		return DashMontageOverride;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	IPRAnimRegistryInterface* ARI = Cast<IPRAnimRegistryInterface>(AvatarActor);
	if (!ARI)
	{
		return nullptr;
	}

	// 방향 계산 및 해당 방향 몽타주 검색
	const FVector LocalDir = GetDashDirection();
	const FGameplayTag DirectionTag = GetDashMontageTag(LocalDir);

	UAnimMontage* DirectionalMontage = ARI->FindMontageByGameplayTag(DirectionTag);
	if (IsValid(DirectionalMontage))
	{
		return DirectionalMontage;
	}

	// 방향별 몽타주 없으면 기본 대시 몽타주로 폴백
	return ARI->FindMontageByGameplayTag(TAG_Montage_Dash);
}

void UPRGA_Dash::ApplyDashMovement()
{
	const FVector DashDirection = GetDashWorldDirection();

	DashMoveTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		NAME_None,
		DashDirection,
		DashStrength,
		DashDuration,
		true,                                              // bIsAdditive
		nullptr,                                            // StrengthOverTime 커브
		ERootMotionFinishVelocityMode::ClampVelocity,
		FVector::ZeroVector,
		0.0f,
		bEnableGravityDuringDash
	);
	
	DashMoveTask->OnFinish.AddDynamic(this, &ThisClass::OnDashMovementFinished);
	
	// 대시 이동 중 이동 입력 제한
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(TAG_State_BlockMovementInput);
	}

	DashMoveTask->ReadyForActivation();
}


void UPRGA_Dash::OnDashMovementFinished()
{
	// 대시 이동 후 이동 입력 제한 해제
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(TAG_State_BlockMovementInput);
	}
}


void UPRGA_Dash::OnMontageCompleted()
{
	K2_EndAbility();
}

void UPRGA_Dash::OnMontageBlendOut()
{
	K2_EndAbility();
}

void UPRGA_Dash::OnMontageCancelled()
{
	K2_EndAbility();
}
