// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeModuleData.h"

float UPRUpgradeModuleData::GetCostForLevel(int32 Level) const
{
	if (Level <= 0)
	{
		return 0.0f;
	}

	switch (CostScalingMode)
	{
	case EUpgradeCostScalingMode::Fixed:
		return BaseCost;

	case EUpgradeCostScalingMode::Linear:
		return BaseCost + (Level - 1) * CostGrowth;

	case EUpgradeCostScalingMode::Exponential:
		return BaseCost * FMath::Pow(CostGrowth, static_cast<float>(Level - 1));

	case EUpgradeCostScalingMode::Custom:
		if (const FRichCurve* Curve = CostCurve.GetRichCurveConst())
		{
			return Curve->Eval(static_cast<float>(Level));
		}
		return BaseCost;
	}

	return BaseCost;
}

FText UPRUpgradeModuleData::GetDisplayName() const
{
	return ActionData ? ActionData->DisplayName : FText::GetEmpty();
}

FText UPRUpgradeModuleData::GetDescription() const
{
	return ActionData ? ActionData->GetFormattedTextWithValuesByIndex(ActionData->Description) : FText::GetEmpty();
}

UTexture2D* UPRUpgradeModuleData::GetIcon() const
{
	return ActionData ? ActionData->Icon : nullptr;
}

int32 UPRUpgradeModuleData::GetMaxStacks() const
{
	return ActionData ? ActionData->MaxStacks : 0;
}

float UPRUpgradeModuleData::GetNextLevelCost(int32 CurrentLevel) const
{
	return GetCostForLevel(CurrentLevel + 1);
}
