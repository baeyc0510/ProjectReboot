// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "PREnvQueryContext_AllEnemies.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPREnvQueryContext_AllEnemies : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
