// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "PROrbitalStrike.generated.h"

class UNiagaraComponent;
class UParticleSystemComponent;
class USphereComponent;

/**
 * 오비탈 스트라이크 액터
 * 3단계 흐름: Telegraph(경고) → Impact VFX(낙하) → Damage(데미지 판정)
 */
UCLASS()
class PROJECTREBOOT_API APROrbitalStrike : public AActor
{
	GENERATED_BODY()

public:
	APROrbitalStrike();

	// 초기화 및 시작 (Ability에서 EffectSpec을 전달받음)
	void InitStrike(const FGameplayEffectSpecHandle& InDamageSpec, FLinearColor Color, float TelegraphDuration, float Radius);

protected:
	/*~ Phase Handlers ~*/

	// Phase 1: 경고 이펙트 시작
	void StartTelegraphPhase(FLinearColor Color, float TelegraphDuration, float Radius);

	// Phase 2: 낙하 이펙트 시작
	void StartImpactPhase();

	// Phase 3: 데미지 판정
	void ExecuteDamage();

protected:
	/*~ Components ~*/

	// 씬 루트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// 경고 이펙트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UNiagaraComponent> TelegraphEffect;

	// 낙하/폭발 이펙트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UParticleSystemComponent> ImpactEffect;

	// 데미지 판정 영역
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageArea;

	/*~ Config ~*/

	// Impact 연출 딜레이 (VFX 재생 후 데미지 판정까지 대기 시간)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float ImpactDelay = 0.01f;

	/*~ State ~*/

	// 페이즈 전환 타이머
	FTimerHandle PhaseTimerHandle;

	// 캐시된 반지름
	float CachedRadius = 300.f;

	// Ability에서 전달받은 데미지 스펙
	FGameplayEffectSpecHandle DamageSpec;
};
