// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"
#include "PRJustDodgeGhost.generated.h"

class UCapsuleComponent;
class UAbilitySystemComponent;


UCLASS()
class PROJECTREBOOT_API APRJustDodgeGhost : public AActor, public IPRCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APRJustDodgeGhost();
	
	virtual void BeginPlay() override;
	void SetOwnerASC(UAbilitySystemComponent* InASC);
	void InitCapsuleSize(UCapsuleComponent* InOriginalCapsule);

	/*~ IPRCombatInterface ~*/
	virtual UCapsuleComponent* GetCombatCapsuleComponent() const override;
	virtual bool IsDead() const override;
	virtual void Die(const FGameplayEffectContextHandle& EffectContext) override;
	virtual void FinishDie() override;
	virtual void OnHit(const FHitResult& HitResult) override;
	
private:
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC;
};
