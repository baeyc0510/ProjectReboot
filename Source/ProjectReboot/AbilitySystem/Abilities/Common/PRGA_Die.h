// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_Die.generated.h"

enum class EPRHitDirection : uint8;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Die : public UPRGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPRGA_Die();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UGA_Death Interfaces ~*/
	
	// 사망 상태 GE 적용
	void ApplyDeathState(const FGameplayEventData* TriggerEventData);
	
	void PlayDeathMontage(UAnimMontage* MontageToPlay);

	UAnimMontage* GetDeathMontage( const FGameplayEventData* TriggerEventData) const;
	
	FGameplayTag GetDeathMontageTagByDirection(EPRHitDirection HitDirection) const;
	
	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();

	// 액터 사망 완료 처리
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnDeathFinished();

	UFUNCTION(BlueprintCallable)
	void FinishDie();
protected:
	// 사망 상태 GE (State.Dead 태그 부여)
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TSubclassOf<UGameplayEffect> DeathStateEffectClass;

	// 사망 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontageOverride;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float MontagePlayRate = 1.0f;
	
private:
	// 현재 어빌리티 핸들 (EndAbility 호출용)
	FGameplayAbilitySpecHandle CurrentSpecHandle;
	
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;
};
