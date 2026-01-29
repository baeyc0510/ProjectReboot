// PRGA_Interact.cpp
#include "PRGA_Interact.h"

#include "ProjectReboot/Interaction/PRInteractionComponent.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRGA_Interact::UPRGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 사망 중에는 상호작용 방지
	ActivationBlockedTags.AddTag(TAG_State_Dead);

	// 입력 기반 발동
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
}

void UPRGA_Interact::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	// 부여 시 상호작용 컴포넌트 생성
	
	AActor* OwnerActor = ActorInfo->AvatarActor.Get();
	UPRInteractionComponent* ExistingComponent = OwnerActor->FindComponentByClass<UPRInteractionComponent>();
	if (IsValid(ExistingComponent))
	{
		ExistingComponent->InitializeSettings(InteractionDistance, InteractionRadius);
		CreatedInteractionComponent = ExistingComponent;
		return;
	}

	UPRInteractionComponent* NewComponent = NewObject<UPRInteractionComponent>(OwnerActor, UPRInteractionComponent::StaticClass(), TEXT("InteractionComponent"));
	if (!IsValid(NewComponent))
	{
		return;
	}

	NewComponent->RegisterComponent();
	NewComponent->InitializeSettings(InteractionDistance, InteractionRadius);
	CreatedInteractionComponent = NewComponent;
}

void UPRGA_Interact::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// 제거 시 상호작용 컴포넌트 정리
	if (CreatedInteractionComponent.IsValid())
	{
		CreatedInteractionComponent->DestroyComponent();
		CreatedInteractionComponent.Reset();
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UPRGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (IsValid(OwnerActor))
	{
		UPRInteractionComponent* InteractionComponent = OwnerActor->FindComponentByClass<UPRInteractionComponent>();
		if (IsValid(InteractionComponent))
		{
			InteractionComponent->TryInteract();
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
