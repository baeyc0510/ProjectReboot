// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PRCombatInterface.generated.h"

class UCapsuleComponent;
struct FGameplayEffectContextHandle;
struct FHitResult;

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPRCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 전투 관련 인터페이스
 * 사망 처리 및 전투 상태 관리
 */
class PROJECTREBOOT_API IPRCombatInterface
{
	GENERATED_BODY()

public:
	// 피격 판정 대상 CapsuleComponent 반환
	virtual UCapsuleComponent* GetCombatCapsuleComponent() const = 0;
	
	/*~ Death ~*/
	// 현재 사망 상태인지 반환
	UFUNCTION(BlueprintCallable, Category = "Combat|Death")
	virtual bool IsDead() const = 0;
	
	// 사망 처리 시작 (Health가 0 이하로 떨어졌을 때 호출)
	virtual void Die(const FGameplayEffectContextHandle& EffectContext) = 0;

	// 사망 완료 후 (Ragdoll, 입력 등 처리)
	virtual void FinishDie() = 0;

	/*~ Hit ~*/
	// 피격 이벤트 처리
	virtual void OnHit(const FHitResult& HitResult) = 0;
	
};