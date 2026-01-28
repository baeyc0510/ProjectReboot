// PRProportionalNavigationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PRProportionalNavigationComponent.generated.h"

/**
 * 비례항법(Proportional Navigation) 기반 유도 미사일 이동 컴포넌트
 *
 * 기존 UProjectileMovementComponent의 단순 호밍 대신,
 * 시선각(LOS) 변화율에 비례한 가속도를 적용하여 효율적인 요격 궤적 생성
 *
 * 가속도 = N × Vc × (dλ/dt)
 * - N: 항법 상수 (NavigationConstant)
 * - Vc: 접근 속도 (Closing Velocity)
 * - dλ/dt: 시선각 변화율
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRProportionalNavigationComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	UPRProportionalNavigationComponent();

	// 비례항법 유도 활성화
	UFUNCTION(BlueprintCallable, Category = "Proportional Navigation")
	void EnableProportionalNavigation(bool bEnable);

	// 유도 타겟 컴포넌트 설정
	UFUNCTION(BlueprintCallable, Category = "Proportional Navigation")
	void SetNavigationTarget(USceneComponent* TargetComponent);

	// 비례항법 활성화 여부
	UFUNCTION(BlueprintPure, Category = "Proportional Navigation")
	bool IsProportionalNavigationEnabled() const { return bUseProportionalNavigation; }

protected:
	/*~ UActorComponent Interface ~*/
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*~ 비례항법 계산 ~*/

	// 비례항법 가속도 계산 및 적용
	void ApplyProportionalNavigation(float DeltaTime);

	// 시선각(LOS) 벡터 계산 (미사일 -> 타겟)
	FVector ComputeLOSVector() const;

	// 시선각 변화율 계산
	FVector ComputeLOSRate(const FVector& CurrentLOS, float DeltaTime) const;

	// 접근 속도 계산
	float ComputeClosingVelocity(const FVector& LOSDirection) const;

public:
	/*~ 비례항법 파라미터 ~*/

	// 비례항법 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation")
	bool bUseProportionalNavigation = true;

	// 항법 상수 (N) - 높을수록 공격적인 기동
	// 일반적으로 3~5 사용, 실제 미사일은 3~4
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float NavigationConstant = 4.0f;

	// 최대 유도 가속도 (cm/s²)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation", meta = (ClampMin = "0.0"))
	float MaxNavigationAcceleration = 10000.0f;

	// 방향 보정 게인 (Augmented PN) - LOS 변화율이 작을 때 타겟 방향으로 추가 보정
	// 0이면 순수 PN, 높을수록 타겟 방향 추종 강화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirectionCorrectionGain = 0.3f;

protected:
	// 유도 타겟 컴포넌트
	UPROPERTY(BlueprintReadOnly, Category = "Proportional Navigation")
	TWeakObjectPtr<USceneComponent> NavigationTargetComponent;

private:
	// 이전 프레임의 LOS 벡터 (시선각 변화율 계산용)
	FVector PreviousLOSVector = FVector::ZeroVector;

	// 이전 프레임의 타겟 위치 (타겟 속도 추정용)
	FVector PreviousTargetLocation = FVector::ZeroVector;

	// 첫 프레임 여부 (초기화용)
	bool bIsFirstFrame = true;
};
