// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"
#include "PRCharacterBase.generated.h"

class UPRMontageSet;
class UPRCommonAttributeSet;
class UPRAbilitySystemComponent;
class UAudioComponent;
class USoundBase;
struct FHitResult;

UCLASS()
class PROJECTREBOOT_API APRCharacterBase : public ACharacter, public IAbilitySystemInterface, public IPRCombatInterface, public IPRAnimRegistryInterface
{
	GENERATED_BODY()

public:
	APRCharacterBase();
	
	/*~ IAbilitySystemInterface ~*/
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/*~ IPRCombatInterface ~*/
	virtual UCapsuleComponent* GetCombatCapsuleComponent() const override;
	virtual bool IsDead() const override;
	virtual void Die(const FGameplayEffectContextHandle& EffectContext) override;
	virtual void FinishDie() override;
	// 피격 이벤트 처리
	virtual void OnHit(const FHitResult& HitResult) override;
	
	/*~ IPRAnimRegistryInterface Interface ~*/
	virtual UAnimMontage* FindMontageByGameplayTag(const FGameplayTag& MontageTag) const override;
	
	/*~ APRCharacterBase Interface ~*/
	float GetBaseMoveSpeed() const {return BaseMoveSpeed;}
	float GetMaxMoveSpeed() const;

	void AddDefaultAbilitySystemTags() const;
	virtual void HandleCollisionAndMovementOnDeath();

	/*~ Loop Sound ~*/
	// 태그 기반 루프 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* PlayLoopSound(FGameplayTag SoundTag, USoundBase* Sound);

	// 태그 기반 루프 사운드 정지 (FadeOutDuration > 0이면 페이드아웃)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopLoopSound(FGameplayTag SoundTag, float FadeOutDuration = 0.0f);

	// 모든 루프 사운드 정지 (FadeOutDuration > 0이면 페이드아웃)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllLoopSounds(float FadeOutDuration = 0.0f);
	
protected:
	/** MovementConfigs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BaseMoveSpeed = 500.f;
	
	/** Animations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Animation")
	TObjectPtr<UPRMontageSet> MontageSet;
	
	/** AbilitySystem */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR AbilitySystem")
	FGameplayTagContainer DefaultAbilitySystemTags;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UCapsuleComponent> CombatCapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRAbilitySystemComponent> AbilitySystem;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRCommonAttributeSet> CommonAttributeSet;

	/*~ Loop Sound ~*/
	// 태그별 활성 루프 사운드 컴포넌트
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UAudioComponent>> ActiveLoopSounds;
};
