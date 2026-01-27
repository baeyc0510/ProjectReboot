// PRMissileProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRMissileProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
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

protected:
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	// 발사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

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
};
