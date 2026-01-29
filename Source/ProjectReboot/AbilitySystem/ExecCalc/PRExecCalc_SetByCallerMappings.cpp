// Fill out your copyright notice in the Description page of Project Settings.


#include "PRExecCalc_SetByCallerMappings.h"

void UPRExecCalc_SetByCallerMappings::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	for (const TPair<FGameplayTag, FPRSetByCallerAttributeMapEntry>& KVP : SetByCallerAttributeMap)
	{
		auto& SetByCallerTag = KVP.Key;
		if (!SetByCallerTag.IsValid())
		{
			continue;
		}
		
		auto& AttributeMapEntry = KVP.Value;
		if (!AttributeMapEntry.Attribute.IsValid())
		{
			continue;
		}
		
		TryApplySetByCaller(Spec,SetByCallerTag,AttributeMapEntry.Attribute, OutExecutionOutput, AttributeMapEntry.ModOp);
	}
}

void UPRExecCalc_SetByCallerMappings::TryApplySetByCaller(const FGameplayEffectSpec& Spec,
                                                          const FGameplayTag& SetByCallerTag, const FGameplayAttribute& TargetAttribute,
                                                          FGameplayEffectCustomExecutionOutput& OutExecutionOutput, EGameplayModOp::Type ModOp) const
{
	const float* MagnitudePtr = Spec.SetByCallerTagMagnitudes.Find(SetByCallerTag);
	if (!MagnitudePtr)
	{
		return;
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		TargetAttribute,
		ModOp,
		*MagnitudePtr));
}
