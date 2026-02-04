// PRHoundAOEZone.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "PRHoundAOEZone.generated.h"

class UNiagaraComponent;
class USphereComponent;
class UAbilitySystemComponent;

/**
 * 하운드 AOE 영역 액터
 * 영역 내 대상에 Duration GE를 적용하고, 영역 이탈 시 GE를 제거
 */
UCLASS()
class PROJECTREBOOT_API APRHoundAOEZone : public AActor
{
	GENERATED_BODY()

public:
	APRHoundAOEZone();

	// 초기화 및 시작 (Ability에서 DamageSpec 전달)
	void InitZone(const FGameplayEffectSpecHandle& InDamageSpec, FLinearColor Color,
		float TelegraphDuration, float Radius, float DamageDuration);

protected:
	/*~ Phase Handlers ~*/

	// Phase 1: 경고 이펙트 시작
	void StartTelegraphPhase(FLinearColor Color, float TelegraphDuration, float Radius);

	// Phase 2: 낙하 이펙트 및 화염 VFX 시작
	void StartImpactPhase();

	// Phase 3: 틱 데미지 영역 활성화
	void StartDamageZonePhase();

	// Phase 4: 정리 및 파괴 예약
	void CleanupZone();

	// 데미지 영역 오버랩 콜백
	UFUNCTION()
	void OnDamageAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDamageAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 대상에 데미지 GE 적용
	void ApplyDamageEffectToActor(AActor* TargetActor);

	// 대상에서 데미지 GE 제거
	void RemoveDamageEffectFromActor(AActor* TargetActor);

	// 모든 적용된 GE 제거
	void RemoveAllAppliedEffects();

protected:
	/*~ Components ~*/

	// 씬 루트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// 경고 이펙트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UNiagaraComponent> TelegraphEffect;

	// 낙하 이펙트 (하늘에서 불꽃 떨어지는 VFX)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UNiagaraComponent> ImpactEffect;

	// 데미지 판정 영역
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageArea;

	/*~ Config ~*/

	// Impact 연출 딜레이 (낙하 VFX 재생 후 데미지 시작까지 대기 시간)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float ImpactDelay = 0.3f;

	// VFX 잔여 후 파괴까지 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float CleanupLifeSpan = 2.0f;

	/*~ State ~*/

	// 페이즈 전환 타이머
	FTimerHandle PhaseTimerHandle;

	// 데미지 영역 지속 타이머
	FTimerHandle DamageZoneTimerHandle;

	// 캐시된 반지름
	float CachedRadius = 300.f;

	// 캐시된 데미지 지속 시간
	float CachedDamageDuration = 5.f;

	// Ability에서 전달받은 데미지 스펙
	FGameplayEffectSpecHandle DamageSpec;

	// 적용된 GE 핸들 추적 (영역 이탈 시 제거용)
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> AppliedEffectHandles;
};
