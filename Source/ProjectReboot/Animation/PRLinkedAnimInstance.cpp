// Fill out your copyright notice in the Description page of Project Settings.


#include "PRLinkedAnimInstance.h"

#include "PRAnimInstance.h"

void UPRLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	MainAnimInstance = Cast<UPRAnimInstance>(Blueprint_GetMainAnimInstance());
}

void UPRLinkedAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!IsValid(MainAnimInstance))
	{
		return;
	}
	
	bIsSprint = MainAnimInstance->bIsSprint;
	F_OrientationAngle = MainAnimInstance->F_OrientationAngle;
	R_OrientationAngle = MainAnimInstance->R_OrientationAngle;
	B_OrientationAngle = MainAnimInstance->B_OrientationAngle;
	L_OrientationAngle = MainAnimInstance->L_OrientationAngle;
	LeanDirection = MainAnimInstance->LeanDirection;
	CardinalDirection = MainAnimInstance->CardinalDirection;
}
