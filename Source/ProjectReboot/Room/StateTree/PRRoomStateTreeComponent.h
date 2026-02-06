// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "PRRoomStateTreeComponent.generated.h"


UCLASS(meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRRoomStateTreeComponent : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPRRoomStateTreeComponent();
	
	UFUNCTION(BlueprintCallable, Category = "PR|StateTree")
	void SetStateTreeAsset(UStateTree* InStateTree);
};
