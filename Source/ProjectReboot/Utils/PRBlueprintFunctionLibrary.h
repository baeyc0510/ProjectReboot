// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PRBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Mesh Utils")
	static TArray<UMaterialInstanceDynamic*> GetAllDynamicMaterials(UMeshComponent* TargetMesh, bool bCreateIfMissing = true);
};
