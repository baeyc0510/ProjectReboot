// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "PRStateTreeEvaluatorBase.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, Abstract)
class PROJECTREBOOT_API UPRStateTreeEvaluatorBase : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()
	
public:
	UPRStateTreeEvaluatorBase(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};