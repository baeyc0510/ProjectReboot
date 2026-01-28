// PRMissileProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRMissileProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPRProportionalNavigationComponent;
class UGameplayEffect;
class UAbilitySystemComponent;
class UNiagaraSystem;

/**
 * 미사일 발사체
 * - ProjectileMovementComponent 기반 비행
 * - 충돌 또는 근접 폭발 시 AOE 데미지
 * - HomingTarget 설정 시 유도 비행
 */
UCLASS()
class PROJECTREBOOT_API APRMissileProjectile : public AActor
{
	GENERATED_BODY()

public:
	APRMissileProjectile();

	virtual void Tick(float DeltaTime) override;

	// 유도 타겟 설정
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetHomingTarget(AActor* Target);

	// 폭발 반경 설정
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetExplosionRadius(float Radius);

	// 데미지 설정
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetDamageEffectClass(TSubclassOf<UGameplayEffect> EffectClass);

	// Instigator ASC 설정 (데미지 적용 시 Source로 사용)
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetInstigatorASC(UAbilitySystemComponent* ASC);

	// 발사 속도 및 방향 설정
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void LaunchInDirection(const FVector& Direction, float Speed);

	// 항법 상수 설정 (비례항법 공격성, 3~5 권장)
	UFUNCTION(BlueprintCallable, Category = "Missile|Homing")
	void SetNavigationConstant(float N);

	// 최대 유도 가속도 설정
	UFUNCTION(BlueprintCallable, Category = "Missile|Homing")
	void SetMaxNavigationAcceleration(float MaxAcceleration);

	// 최대 사거리 설정 (0 이하면 무제한)
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetMaxRange(float Range);

protected:
	virtual void BeginPlay() override;

	// 충돌 처리
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 폭발 및 AOE 데미지 처리
	void Explode();

	// 폭발 이펙트 스폰
	void SpawnExplosionEffect();

	// 범위 내 타겟에게 데미지 적용
	void ApplyAOEDamage();

	// 근접 폭발 체크 (유도 미사일용)
	void CheckProximityDetonation();

	// 최대 사거리 초과 체크
	void CheckMaxRangeDetonation();

protected:
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	// 발사체 이동 컴포넌트 (비례항법 유도)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPRProportionalNavigationComponent> ProjectileMovement;

	// 메시 컴포넌트 (선택적, BP에서 설정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	// 폭발 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	float ExplosionRadius = 200.0f;

	// 근접 폭발 거리 (HomingTarget 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	float ProximityDetonationRadius = 100.0f;

	// 데미지 GameplayEffect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 폭발 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	// 유도 타겟
	UPROPERTY(BlueprintReadOnly, Category = "Missile|Homing")
	TWeakObjectPtr<AActor> HomingTarget;

	// Instigator의 AbilitySystemComponent (데미지 Source)
	UPROPERTY(BlueprintReadOnly, Category = "Missile|Damage")
	TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC;

	// 폭발 여부 (중복 폭발 방지)
	bool bHasExploded = false;

	// 최대 사거리 (0 이하면 무제한)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile")
	float MaxRange = 5000.0f;

	// 발사 위치 (사거리 계산용)
	FVector LaunchLocation = FVector::ZeroVector;
};
