// PRGA_Interact.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Interact.generated.h"

/**
 * 상호작용 어빌리티
 * - 입력 기반 발동
 * - InteractionComponent를 통해 상호작용 실행
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Interact : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRGA_Interact();

	/*~ UGameplayAbility Interface ~*/
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
public:
	// 상호작용 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionDistance = 300.0f;

	// 상호작용 반경
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRadius = 50.0f;

private:
	// 생성된 상호작용 컴포넌트
	UPROPERTY()
	TWeakObjectPtr<class UPRInteractionComponent> CreatedInteractionComponent;
};
