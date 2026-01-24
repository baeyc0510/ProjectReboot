// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "PRStateTreeConditionBase.generated.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Deprecated,
 * USTRUCT 기반 Condition으로 이동됨. 성능 비교용으로 사용후 제거 예정
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

UCLASS()
class PROJECTREBOOT_API UPRStateTreeConditionBase : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()
	
protected:
	virtual bool NativeTestCondition(FStateTreeExecutionContext& Context) const {return false;}
	
private:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override final;
};