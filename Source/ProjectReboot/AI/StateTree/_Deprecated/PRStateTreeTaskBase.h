// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "PRStateTreeTaskBase.generated.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Deprecated,
 * USTRUCT 기반 Task로 이동됨. 성능 비교용으로 사용후 제거 예정
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

UCLASS(NotBlueprintable, Abstract)
class PROJECTREBOOT_API UPRStateTreeTaskBase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual void OnEnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) {}
	virtual void NativeReceivedTick(FStateTreeExecutionContext& Context, const float DeltaTime) {}
	virtual void OnExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) {}
	virtual void OnStateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) {}
	
private:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override final;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override final;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override final;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) override final;
};