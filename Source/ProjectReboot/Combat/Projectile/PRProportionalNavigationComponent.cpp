#include "PRProportionalNavigationComponent.h"

UPRProportionalNavigationComponent::UPRProportionalNavigationComponent()
{
    // 비례항법 기동을 위해 기존 호밍 및 물리 옵션 최적화
    bIsHomingProjectile = false;
    HomingAccelerationMagnitude = 0.0f;
    bRotationFollowsVelocity = true;
    ProjectileGravityScale = 0.0f;
}

void UPRProportionalNavigationComponent::EnableProportionalNavigation(bool bEnable)
{
    bUseProportionalNavigation = bEnable;
    if (!bEnable) bIsFirstFrame = true;
}

void UPRProportionalNavigationComponent::SetNavigationTarget(USceneComponent* TargetComponent)
{
    NavigationTargetComponent = TargetComponent;
    bIsFirstFrame = true;
}

void UPRProportionalNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // 이동 업데이트 수행 전 유도 가속도를 계산하여 Velocity 수정
    if (bUseProportionalNavigation && NavigationTargetComponent.IsValid())
    {
        ApplyProportionalNavigation(DeltaTime);
    }

    // 기본 이동 업데이트
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPRProportionalNavigationComponent::ApplyProportionalNavigation(float DeltaTime)
{
    if (DeltaTime <= SMALL_NUMBER || !UpdatedComponent)
    {
        return;
    }

    FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
    FVector TargetLocation = NavigationTargetComponent->GetComponentLocation();
    FVector CurrentLOS = TargetLocation - CurrentLocation;
    float Distance = CurrentLOS.Size();

    // 타겟과 너무 가까우면 마지막 유도 방향을 유지
    if (Distance < MinGuidanceDistance)
    {
        return;
    }

    // 타겟의 현재 속도 추정
    FVector TargetVelocity = FVector::ZeroVector;
    if (AActor* TargetActor = NavigationTargetComponent->GetOwner())
    {
        TargetVelocity = TargetActor->GetVelocity();
        // 타겟의 GetVelocity가 유효하지 않을 경우 위치 변화량으로 보정
        if (TargetVelocity.IsNearlyZero() && !bIsFirstFrame)
        {
            TargetVelocity = (TargetLocation - PreviousTargetLocation) / DeltaTime;
        }
    }

    FVector CurrentLOSDirection = CurrentLOS.GetSafeNormal();

    // 초기 프레임 데이터 샘플링 (변화율 계산을 위한 준비)
    if (bIsFirstFrame)
    {
        PreviousLOSDirection = CurrentLOSDirection;
        PreviousTargetLocation = TargetLocation;
        bIsFirstFrame = false;
        return;
    }

    // 각속도 벡터 계산
    FVector LOSRateVector = ComputeLOSRateVector(CurrentLOSDirection, DeltaTime);

    // 접근 속도 계산
    float Vc = ComputeClosingVelocity(CurrentLOSDirection, TargetVelocity);

    // 비례항법 기본 공식 적용 (Accel = N * Vc * LOS_Rate)
    FVector PNAcceleration = NavigationConstant * Vc * LOSRateVector;

    // 가속도 한계치 제한: 급격한 화면 전환 시 발생하는 비정상적 기동 방지
    if (PNAcceleration.Size() > MaxNavigationAcceleration)
    {
        PNAcceleration = PNAcceleration.GetSafeNormal() * MaxNavigationAcceleration;
    }

    // 현재 속력을 유지하며 가속도에 의한 방향만 수정
    float CurrentSpeed = Velocity.Size();
    Velocity += PNAcceleration * DeltaTime;
    Velocity = Velocity.GetSafeNormal() * CurrentSpeed;

    // 상태값 저장
    PreviousLOSDirection = CurrentLOSDirection;
    PreviousTargetLocation = TargetLocation;
}

FVector UPRProportionalNavigationComponent::ComputeLOSRateVector(const FVector& CurrentLOSDirection, float DeltaTime) const
{
    // 단위 벡터의 변화율을 통해 시선 방향의 회전량 추출
    FVector Rate = (CurrentLOSDirection - PreviousLOSDirection) / DeltaTime;

    // 급작스러운 발사 오차나 화면 전환 시 미사일이 튀는 것을 막기 위한 초당 회전 가능한 각속도의 상한선
    const float MaxRateMagnitude = 15.0f; 
    if (Rate.Size() > MaxRateMagnitude)
    {
        Rate = Rate.GetSafeNormal() * MaxRateMagnitude;
    }

    return Rate;
}

float UPRProportionalNavigationComponent::ComputeClosingVelocity(const FVector& LOSDirection, const FVector& TargetVelocity) const
{
    // 상대 속도 계산
    FVector RelativeVelocity = Velocity - TargetVelocity;

    // 시선 방향으로의 접근 속도 성분 추출
    float Vc = -FVector::DotProduct(RelativeVelocity, LOSDirection);

    // 타겟을 지나치더라도 유도력을 잃지 않도록 최소 접근 속도를 현재 속도의 50%로 유지
    return FMath::Max(Vc, Velocity.Size() * 0.5f);
}