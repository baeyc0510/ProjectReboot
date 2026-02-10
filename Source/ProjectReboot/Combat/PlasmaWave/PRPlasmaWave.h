// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "PRPlasmaWave.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 플라즈마 웨이브 발사체
 * 지면을 따라 직선으로 전진하며 접촉한 액터에게 데미지를 적용
 */
UCLASS()
class PROJECTREBOOT_API APRPlasmaWave : public AActor
{
	GENERATED_BODY()

public:
	APRPlasmaWave();

	virtual void Tick(float DeltaTime) override;

	// 웨이브 초기화 (방향, 데미지 스펙 설정)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void InitWave(const FVector& Direction, const FGameplayEffectSpecHandle& InDamageSpec);

protected:
	virtual void BeginPlay() override;

	// Overlap 콜백
	UFUNCTION()
	void OnWaveOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// 지면 높이 추적
	void TraceGround(float DeltaTime);

protected:
	/*~ Components ~*/

	// 씬 루트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// 데미지 판정 영역 (박스: 웨이브 형태)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> DamageBox;

	// 웨이브 이펙트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UNiagaraComponent> WaveEffect;

	/*~ Config ~*/

	// 웨이브 이펙트 Niagara 시스템
	UPROPERTY(EditDefaultsOnly, Category = "Config|VFX")
	TObjectPtr<UNiagaraSystem> WaveNiagaraSystem;

	// 이동 속도 (cm/s)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MoveSpeed = 1200.f;

	// 최대 이동 거리
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MaxDistance = 3000.f;

	// 지면 추적 레이캐스트 높이
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float GroundTraceHeight = 500.f;

	// 데미지 박스 크기 (Half Extent)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FVector DamageBoxExtent = FVector(50.f, 150.f, 100.f);

	/*~ State ~*/

	// 이동 방향
	FVector MoveDirection = FVector::ForwardVector;

	// 이동한 총 거리
	float TraveledDistance = 0.f;

	// Ability에서 전달받은 데미지 스펙
	FGameplayEffectSpecHandle DamageSpec;

	// 중복 히트 방지용 캐시
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
