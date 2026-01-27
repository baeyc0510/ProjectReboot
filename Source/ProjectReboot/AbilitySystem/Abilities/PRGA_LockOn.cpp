// PRGA_LockOn.cpp
#include "PRGA_LockOn.h"
#include "AbilitySystemComponent.h"
#include "ProjectReboot/Combat/Targeting/PRTargetLockComponent.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_LockOn::UPRGA_LockOn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 패시브 어빌리티 - 부여 시 자동 활성화
	ActivationPolicy = EPRAbilityActivationPolicy::OnGiven;

	// 기본 컴포넌트 클래스 설정
	TargetLockComponentClass = UPRTargetLockComponent::StaticClass();

	// 기본 락온 상태 태그 설정
	LockOnStateTag = TAG_State_Aiming;
}

void UPRGA_LockOn::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	// TargetLockComponent 생성 및 부착
	UPRTargetLockComponent* LockComponent = CreateAndAttachLockComponent(AvatarActor);
	if (IsValid(LockComponent))
	{
		// 현재 상태에 맞게 락온 활성화 갱신
		const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const bool bHasTag = IsValid(ASC) ? ASC->HasMatchingGameplayTag(LockOnStateTag) : false;
		LockComponent->SetLockOnEnabled(bHasTag);
	}
}

void UPRGA_LockOn::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	UnbindFromASC();

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		RemoveLockComponent(ActorInfo->AvatarActor.Get());
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UPRGA_LockOn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo)
	{
		return;
	}

	BindToASC(ActorInfo->AbilitySystemComponent.Get());
	RefreshLockOnEnabled();
}

void UPRGA_LockOn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnbindFromASC();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_LockOn::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();

	if (!IsValid(InASC))
	{
		return;
	}

	BoundASC = InASC;
	LockOnStateTagHandle = InASC->RegisterGameplayTagEvent(LockOnStateTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPRGA_LockOn::HandleLockOnStateTagChanged);
}

void UPRGA_LockOn::UnbindFromASC()
{
	if (BoundASC.IsValid() && LockOnStateTagHandle.IsValid())
	{
		BoundASC->UnregisterGameplayTagEvent(LockOnStateTagHandle, LockOnStateTag, EGameplayTagEventType::NewOrRemoved);
	}
	LockOnStateTagHandle.Reset();
	BoundASC.Reset();
}

void UPRGA_LockOn::HandleLockOnStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (CreatedLockComponent.IsValid())
	{
		CreatedLockComponent->SetLockOnEnabled(NewCount > 0);
	}
}

void UPRGA_LockOn::RefreshLockOnEnabled()
{
	if (!CreatedLockComponent.IsValid() || !BoundASC.IsValid())
	{
		return;
	}

	const bool bHasTag = BoundASC->HasMatchingGameplayTag(LockOnStateTag);
	CreatedLockComponent->SetLockOnEnabled(bHasTag);
}

UPRTargetLockComponent* UPRGA_LockOn::CreateAndAttachLockComponent(AActor* Owner)
{
	if (!IsValid(Owner) || !TargetLockComponentClass)
	{
		return nullptr;
	}

	// 이미 컴포넌트가 있는지 확인
	UPRTargetLockComponent* ExistingComponent = Owner->FindComponentByClass<UPRTargetLockComponent>();
	if (IsValid(ExistingComponent))
	{
		CreatedLockComponent = ExistingComponent;
		return ExistingComponent;
	}

	// 새 컴포넌트 생성
	UPRTargetLockComponent* NewComponent = NewObject<UPRTargetLockComponent>(Owner, TargetLockComponentClass);
	if (!IsValid(NewComponent))
	{
		return nullptr;
	}

	// 어빌리티에서 설정한 값으로 오버라이드
	NewComponent->OverrideLockOnRange(LockOnRange);
	NewComponent->OverrideLockOnConeAngle(LockOnConeAngle);
	NewComponent->OverrideLockOnTime(LockOnTime);

	// 컴포넌트 등록
	NewComponent->RegisterComponent();

	CreatedLockComponent = NewComponent;
	return NewComponent;
}

void UPRGA_LockOn::RemoveLockComponent(AActor* Owner)
{
	if (!IsValid(Owner))
	{
		return;
	}

	// 생성한 컴포넌트 제거
	if (CreatedLockComponent.IsValid())
	{
		UPRTargetLockComponent* LockComponent = CreatedLockComponent.Get();

		// 락온 비활성화 및 정리
		LockComponent->SetLockOnEnabled(false);
		LockComponent->ClearAllLocks();

		// 컴포넌트 제거
		LockComponent->DestroyComponent();
		CreatedLockComponent.Reset();
	}
}
