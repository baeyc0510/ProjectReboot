// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRAnimationTypes.h"
#include "Animation/AnimInstance.h"
#include "PRLinkedAnimInstance.generated.h"

class UPRAnimInstance;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPRAnimInstance> MainAnimInstance;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsSprint;
	
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LeanDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECardinalDirection CardinalDirection;
};
