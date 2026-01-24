// Fill out your copyright notice in the Description page of Project Settings.


#include "PRStateTreeConditionBase.h"

bool UPRStateTreeConditionBase::TestCondition(FStateTreeExecutionContext& Context) const
{
	SetCachedInstanceDataFromContext(Context);

	const bool bResult = NativeTestCondition(Context);

	ClearCachedInstanceData();

	return bResult;
}
