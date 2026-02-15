// PRMissileProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
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
class PROJECTREBOOT_API APRMissileProjectile : public AActor, public IPRPrewarmInterface
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

	// 무기 슬롯 태그 설정 (GCN에서 무기 인스턴스 조회용)
	UFUNCTION(BlueprintCallable, Category = "Missile")
	void SetWeaponSlotTag(FGameplayTag SlotTag);

	// 추가 폭발 설정 (Scatter)
	UFUNCTION(BlueprintCallable, Category = "Missile|Scatter")
	void SetSubExplosion(int32 Count, float Radius, float SpreadRadius, float Interval = 0.08f);

	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 나이아가라 에셋 수집
	virtual void GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const override;

protected:
	virtual void BeginPlay() override;

	// 충돌 처리
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 폭발 및 AOE 데미지 처리
	void Explode();

	// 폭발 이펙트 스폰
	void SpawnExplosionEffect();

	// 지정 위치에 폭발 이펙트 스폰
	void SpawnExplosionEffectAt(const FVector& Location);

	// 범위 내 타겟에게 데미지 적용
	void ApplyAOEDamage();

	// 지정 위치에서 AOE 데미지 적용
	void ApplyAOEDamageAt(const FVector& Location, float Radius);

	// 추가 폭발 처리 (타이머 콜백)
	void ProcessNextSubExplosion();

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	TObjectPtr<USoundBase> ExplosionSFX;

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

	// 무기 슬롯 태그 (GCN에서 무기 인스턴스 조회용)
	UPROPERTY(BlueprintReadOnly, Category = "Missile|Damage")
	FGameplayTag WeaponSlotTag;

	/*~ 추가 폭발 (Scatter) ~*/

	// 추가 폭발 개수 (0이면 비활성)
	int32 SubExplosionCount = 0;

	// 추가 폭발 AOE 반경
	float SubExplosionRadius = 100.f;

	// 추가 폭발 산개 거리 (메인 폭발 중심 기준)
	float SubExplosionSpreadRadius = 200.f;

	// 추가 폭발 간격 (초)
	float SubExplosionInterval = 0.08f;

	// 메인 폭발 위치 (추가 폭발 기준점)
	FVector MainExplosionLocation = FVector::ZeroVector;

	// 미리 생성된 추가 폭발 위치 목록
	TArray<FVector> SubExplosionLocations;

	// 현재 추가 폭발 인덱스
	int32 CurrentSubExplosionIndex = 0;

	// 추가 폭발 타이머 핸들
	FTimerHandle SubExplosionTimerHandle;
};
