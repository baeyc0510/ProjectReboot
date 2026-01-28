// PRProportionalNavigationComponent.cpp
#include "PRProportionalNavigationComponent.h"

UPRProportionalNavigationComponent::UPRProportionalNavigationComponent()
{
	// 기본 호밍 비활성화 (비례항법으로 대체)
	bIsHomingProjectile = false;
	HomingAccelerationMagnitude = 0.0f;

	// 속도 방향으로 회전
	bRotationFollowsVelocity = true;

	// 중력 비활성화
	ProjectileGravityScale = 0.0f;
}

void UPRProportionalNavigationComponent::EnableProportionalNavigation(bool bEnable)
{
	bUseProportionalNavigation = bEnable;

	if (!bEnable)
	{
		// 비활성화 시 상태 초기화
		bIsFirstFrame = true;
		PreviousLOSVector = FVector::ZeroVector;
		PreviousTargetLocation = FVector::ZeroVector;
	}
}

void UPRProportionalNavigationComponent::SetNavigationTarget(USceneComponent* TargetComponent)
{
	NavigationTargetComponent = TargetComponent;

	// 타겟 변경 시 상태 초기화
	bIsFirstFrame = true;
	PreviousLOSVector = FVector::ZeroVector;
	PreviousTargetLocation = FVector::ZeroVector;
}

void UPRProportionalNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// 비례항법 적용 (부모 Tick 전에 가속도 계산)
	if (bUseProportionalNavigation && NavigationTargetComponent.IsValid())
	{
		ApplyProportionalNavigation(DeltaTime);
	}

	// 부모 클래스의 이동 처리
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPRProportionalNavigationComponent::ApplyProportionalNavigation(float DeltaTime)
{
	if (DeltaTime <= SMALL_NUMBER)
	{
		return;
	}

	// 현재 LOS 벡터 계산
	FVector CurrentLOS = ComputeLOSVector();
	if (CurrentLOS.IsNearlyZero())
	{
		return;
	}

	// 첫 프레임이면 이전 값 초기화 후 리턴
	if (bIsFirstFrame)
	{
		PreviousLOSVector = CurrentLOS;
		PreviousTargetLocation = NavigationTargetComponent->GetComponentLocation();
		bIsFirstFrame = false;
		return;
	}

	// LOS 방향 (정규화)
	FVector LOSDirection = CurrentLOS.GetSafeNormal();

	// 시선각 변화율 계산 (LOS 회전 각속도 벡터)
	FVector LOSRate = ComputeLOSRate(CurrentLOS, DeltaTime);

	// 접근 속도 계산
	float ClosingVelocity = ComputeClosingVelocity(LOSDirection);

	// 비례항법 가속도 계산
	// a = N × Vc × (LOS 회전축 × LOS 방향) = N × Vc × LOSRate
	// LOSRate는 이미 LOS에 수직인 성분만 포함
	FVector PNAcceleration = NavigationConstant * ClosingVelocity * LOSRate;

	// Augmented PN: 타겟 방향 보정항 추가
	// 미사일 진행 방향과 LOS 방향의 차이를 보정
	FVector VelocityDir = Velocity.GetSafeNormal();
	FVector LOSCorrectionDir = LOSDirection - VelocityDir;

	// LOS에 수직인 성분만 추출 (진행 방향 성분 제거)
	LOSCorrectionDir = LOSCorrectionDir - (FVector::DotProduct(LOSCorrectionDir, VelocityDir) * VelocityDir);

	// 보정 가속도 = 게인 × 속도 × 방향 오차
	FVector CorrectionAcceleration = DirectionCorrectionGain * Velocity.Size() * LOSCorrectionDir;

	// 총 유도 가속도
	FVector NavigationAcceleration = PNAcceleration + CorrectionAcceleration;

	// 최대 가속도 제한
	float AccelMagnitude = NavigationAcceleration.Size();
	if (AccelMagnitude > MaxNavigationAcceleration)
	{
		NavigationAcceleration = NavigationAcceleration.GetSafeNormal() * MaxNavigationAcceleration;
	}

	// 현재 속도 크기 저장
	float OriginalSpeed = Velocity.Size();

	// 가속도를 속도에 적용
	Velocity += NavigationAcceleration * DeltaTime;

	// 속도 크기 유지 (방향만 변경, 속도는 원래 값 유지)
	// 유도 미사일은 추력으로 속도를 유지한다고 가정
	float TargetSpeed = FMath::Min(OriginalSpeed, MaxSpeed);
	Velocity = Velocity.GetSafeNormal() * TargetSpeed;

	// 다음 프레임을 위해 현재 값 저장
	PreviousLOSVector = CurrentLOS;
	PreviousTargetLocation = NavigationTargetComponent->GetComponentLocation();
}

FVector UPRProportionalNavigationComponent::ComputeLOSVector() const
{
	if (!NavigationTargetComponent.IsValid() || !UpdatedComponent)
	{
		return FVector::ZeroVector;
	}

	FVector MissileLocation = UpdatedComponent->GetComponentLocation();
	FVector TargetLocation = NavigationTargetComponent->GetComponentLocation();

	return TargetLocation - MissileLocation;
}

FVector UPRProportionalNavigationComponent::ComputeLOSRate(const FVector& CurrentLOS, float DeltaTime) const
{
	if (PreviousLOSVector.IsNearlyZero() || DeltaTime <= SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}

	// LOS 방향 변화 계산 (각도 변화)
	FVector PrevLOSDir = PreviousLOSVector.GetSafeNormal();
	FVector CurrLOSDir = CurrentLOS.GetSafeNormal();

	// 두 방향 벡터의 외적 = 회전축 × sin(각도)
	FVector CrossProduct = FVector::CrossProduct(PrevLOSDir, CurrLOSDir);

	// 두 방향 벡터의 내적 = cos(각도)
	float DotProduct = FVector::DotProduct(PrevLOSDir, CurrLOSDir);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

	// 각도 변화량 (라디안)
	float AngleChange = FMath::Acos(DotProduct);

	// 회전축 (정규화)
	FVector RotationAxis = CrossProduct.GetSafeNormal();
	if (RotationAxis.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	// 각속도 (rad/s)
	float AngularRate = AngleChange / DeltaTime;

	// LOS 회전 방향으로의 단위 벡터 (LOS에 수직)
	// 가속도는 LOS에 수직인 방향으로 적용되어야 함
	FVector LOSPerpendicular = FVector::CrossProduct(RotationAxis, CurrLOSDir);

	return LOSPerpendicular * AngularRate;
}

float UPRProportionalNavigationComponent::ComputeClosingVelocity(const FVector& LOSDirection) const
{
	if (!NavigationTargetComponent.IsValid())
	{
		return Velocity.Size();
	}

	// 타겟 속도 추정 (이전 위치와 현재 위치로부터)
	FVector TargetVelocity = FVector::ZeroVector;
	if (!PreviousTargetLocation.IsZero())
	{
		// 주의: 이 시점에서 DeltaTime을 직접 알 수 없으므로
		// 타겟이 AActor인 경우 GetVelocity() 사용 시도
		if (AActor* TargetActor = NavigationTargetComponent->GetOwner())
		{
			TargetVelocity = TargetActor->GetVelocity();
		}
	}

	// 상대 속도 = 미사일 속도 - 타겟 속도
	FVector RelativeVelocity = Velocity - TargetVelocity;

	// 접근 속도 = -상대 속도 · LOS 방향
	// (양수면 접근 중, 음수면 멀어지는 중)
	float ClosingVelocity = -FVector::DotProduct(RelativeVelocity, LOSDirection);

	// 최소 접근 속도 보장 (멀어지는 경우에도 유도 유지)
	return FMath::Max(ClosingVelocity, Velocity.Size() * 0.5f);
}
