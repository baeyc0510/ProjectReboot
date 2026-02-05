// Fill out your copyright notice in the Description page of Project Settings.

#include "PRStageConfigData.h"

FPrimaryAssetId UPRStageConfigData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("PRStageConfig"), GetFName());
}
