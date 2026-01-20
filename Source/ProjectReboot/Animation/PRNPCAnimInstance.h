// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRAnimationTypes.h"
#include "Animation/AnimInstance.h"
#include "PRNPCAnimInstance.generated.h"

class UCharacterMovementComponent;

/**
 * NPC 전용 AnimInstance
 * 4방향 이동 및 Strafe 애니메이션 지원
 */
UCLASS()
class PROJECTREBOOT_API UPRNPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/*~ UAnimInstance Interface ~*/
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

private:
	/*~ Internal ~*/
	void UpdateDirection();
protected:
	/*~ Movement Data ~*/
	// 이동 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving = false;
	
	// 이동 방향 (4방향)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECardinalDirection CardinalDirection = ECardinalDirection::Forward;

	// 전방 기준 방향 각도
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float F_OrientationAngle;
	
	// 우측 기준 방향 각도
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float R_OrientationAngle;
	
	// 후방 기준 방향 각도
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float B_OrientationAngle;
	
	// 좌측 기준 방향 각도
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float L_OrientationAngle;
	
	/*~ State Data ~*/
	// Strafe 모드 (플레이어 방향 고정 상태)
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsStrafing = false;

	/*~ Settings ~*/
	// 이동 판정 최소 속도
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float MoveThreshold = 10.f;

private:
	/*~ Cached References ~*/
	UPROPERTY()
	TWeakObjectPtr<APawn> OwningPawn;
	
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	/*~ Cached Data ~*/
	FVector CachedVelocity = FVector::ZeroVector;
	FVector CachedForward = FVector::ForwardVector;
	FRotator CachedRotation = FRotator::ZeroRotator;
	bool bCachedOrientToMovement = true;
	
};