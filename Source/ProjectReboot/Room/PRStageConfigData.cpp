// Fill out your copyright notice in the Description page of Project Settings.

#include "PRStageConfigData.h"

#include "PRThemeData.h"

FPrimaryAssetId UPRStageConfigData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("PRStageConfig"), GetFName());
}

void UPRStageConfigData::GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const
{
}

void UPRStageConfigData::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	if (IsValid(ThemeData))
	{
		OutChildren.Add(ThemeData);
	}
}
