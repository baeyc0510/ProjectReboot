// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "PRStateTreeEvaluatorBase.generated.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Deprecated,
 * USTRUCT 기반 Eavluator 로 이동됨. 성능 비교용으로 사용후 제거 예정
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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