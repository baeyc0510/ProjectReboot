// PRGA_JustDodge.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "ProjectReboot/JustDodge/PRJustDodgeGhost.h"
#include "PRGA_JustDodge.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitGameplayEffectRemoved;
class UGameplayEffect;
class APRJustDodgeGhost;

/**
 * 저스트 회피 어빌리티
 * - Event.Dash 이벤트로 트리거
 * - State.Dodging 태그 부여 (회피 윈도우)
 * - 회피 윈도우 중 Dodgeable 데미지 수신 또는 HitGhost 수신 시 저스트 회피 성공
 * - 성공 시 Event.JustDodge 이벤트 발송 및 보상 GE 적용
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_JustDodge : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_JustDodge();

	/*~ UGameplayAbility Interface ~*/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_JustDodge Interface ~*/

	// 회피 상태 GE 적용
	void ApplyDodgingState();

	// 데미지 이벤트 대기 시작
	void StartWaitingForDamageEvent();

	// 잔상 히트 이벤트 대기 시작
	void StartWaitingForHitGhostEvent();

	// 저스트 회피 잔상 소환
	void SpawnJustDodgeGhost();

	// 저스트 회피 성공 처리
	void HandleJustDodgeSuccess(const FGameplayEventData* DamageEventData);

	// 저스트 회피 성공 이벤트 발송
	void SendJustDodgeEvent(const FGameplayEventData* DamageEventData);

	// 데미지 이벤트 수신 콜백
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);

	// 잔상 히트 이벤트 수신 콜백
	UFUNCTION()
	void OnHitGhostEventReceived(FGameplayEventData Payload);

	// 회피 상태 GE 만료 콜백
	UFUNCTION()
	void OnDodgingStateExpired(const FGameplayEffectRemovalInfo& RemovalInfo);

protected:
	// 회피 상태 GE (State.Dodging 태그 부여, 지속시간 설정)
	UPROPERTY(EditDefaultsOnly, Category = "JustDodge")
	TSubclassOf<UGameplayEffect> DodgingStateEffectClass;

	// 저스트 회피 성공 시 적용할 보상 GE
	UPROPERTY(EditDefaultsOnly, Category = "JustDodge")
	TSubclassOf<UGameplayEffect> JustDodgeRewardEffectClass;

	// 저스트 회피 잔상 액터 클래스
	UPROPERTY(EditDefaultsOnly, Category = "JustDodge")
	TSubclassOf<APRJustDodgeGhost> JustDodgeGhostClass = APRJustDodgeGhost::StaticClass();

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitDamageEventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitHitGhostEventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEffectRemoved> WaitDodgingStateRemovedTask;

	// 회피 상태 GE 핸들
	FActiveGameplayEffectHandle DodgingEffectHandle;

	// 저스트 회피 잔상 액터
	UPROPERTY()
	TObjectPtr<APRJustDodgeGhost> JustDodgeGhost;

	// 저스트 회피 성공 여부
	bool bJustDodgeSucceeded = false;
};
