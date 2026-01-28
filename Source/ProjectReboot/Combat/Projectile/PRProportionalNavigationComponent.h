#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PRProportionalNavigationComponent.generated.h"

UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRProportionalNavigationComponent : public UProjectileMovementComponent
{
    GENERATED_BODY()

public:
    UPRProportionalNavigationComponent();

    UFUNCTION(BlueprintCallable, Category = "Proportional Navigation")
    void EnableProportionalNavigation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Proportional Navigation")
    void SetNavigationTarget(USceneComponent* TargetComponent);

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 비례항법 가속도 계산 및 속도 반영
    void ApplyProportionalNavigation(float DeltaTime);

    // 시선 방향 단위 벡터의 시간 변화율 계산 (각속도 벡터)
    FVector ComputeLOSRateVector(const FVector& CurrentLOSDirection, float DeltaTime) const;

    // 상대 속도를 이용한 접근 속도(Closing Velocity) 계산
    float ComputeClosingVelocity(const FVector& LOSDirection, const FVector& TargetVelocity) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation")
    bool bUseProportionalNavigation = true;

    // 항법 상수 (N): 타겟의 미래 위치를 얼마나 공격적으로 예측할지 결정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float NavigationConstant = 4.0f;

    // 미사일이 물리적으로 허용하는 최대 유도 가속도 (G-Limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation")
    float MaxNavigationAcceleration = 10000.0f;

    // 타겟과 너무 가까울 때 유도가 발산하여 요동치는 것을 방지하는 최소 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proportional Navigation")
    float MinGuidanceDistance = 50.0f;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Proportional Navigation")
    TWeakObjectPtr<USceneComponent> NavigationTargetComponent;

private:
    FVector PreviousLOSDirection = FVector::ZeroVector;
    FVector PreviousTargetLocation = FVector::ZeroVector;
    bool bIsFirstFrame = true;
};