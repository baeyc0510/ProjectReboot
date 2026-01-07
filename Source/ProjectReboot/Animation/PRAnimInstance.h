// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRAnimationTypes.h"
#include "Animation/AnimInstance.h"
#include "PRAnimInstance.generated.h"

class UCharacterMovementComponent;
class APlayerCharacter;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
private:
	void UpdateVelocity();
	void UpdateAcceleration();
	void UpdateDirection();
	void UpdateFlags();
	void UpdateAim();
	void UpdateLean();
protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APlayerCharacter> PlayerCharacter;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;
	
	/*~ Movements ~*/
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float MoveSpeedThreshold = 3.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity2D;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Acceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector LocalVelocity2D;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector LocalAcceleration2D;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float XYSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float CameraX;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECardinalDirection CardinalDirection;
	
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
	
	// 기울어짐 방향
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LeanDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprint;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ELandState LandState;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Aiming")
	float AimPitch;
	
};
