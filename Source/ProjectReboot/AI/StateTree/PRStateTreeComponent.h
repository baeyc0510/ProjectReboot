// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponent.h"
#include "Components/StateTreeComponent.h"
#include "PRStateTreeComponent.generated.h"


UCLASS(meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRStateTreeComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()

public:
	UPRStateTreeComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/*~ UPRStateTreeComponent Interfaces ~*/
	UFUNCTION(BlueprintCallable, Category = "PR|AI")
	void SetStateTreeAsset(UStateTree* InStateTree);
};