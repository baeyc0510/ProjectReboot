// Fill out your copyright notice in the Description page of Project Settings.

#include "PRSTC_IsWithinDistance.h"
#include "StateTreeExecutionContext.h"

bool FPRStateTreeCondition_IsWithinDistance::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.SourceActor) || !IsValid(InstanceData.TargetActor))
	{
		return InstanceData.bInvert;
	}

	const float Distance = FVector::Dist(
		InstanceData.SourceActor->GetActorLocation(),
		InstanceData.TargetActor->GetActorLocation()
	);

	const bool bIsWithin = Distance <= InstanceData.MaxDistance;
	return InstanceData.bInvert ? !bIsWithin : bIsWithin;
}
