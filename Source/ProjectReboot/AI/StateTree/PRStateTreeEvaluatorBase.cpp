// Fill out your copyright notice in the Description page of Project Settings.


#include "PRStateTreeEvaluatorBase.h"

UPRStateTreeEvaluatorBase::UPRStateTreeEvaluatorBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bHasTreeStart = false;
	bHasTreeStop = false;
	bHasTick = false;
}

void UPRStateTreeEvaluatorBase::TreeStart(FStateTreeExecutionContext& Context)
{
	SetCachedInstanceDataFromContext(Context);
}

void UPRStateTreeEvaluatorBase::TreeStop(FStateTreeExecutionContext& Context)
{
	ClearCachedInstanceData();
}

void UPRStateTreeEvaluatorBase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
}
