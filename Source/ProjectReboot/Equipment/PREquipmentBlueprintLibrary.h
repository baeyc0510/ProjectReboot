// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PREquipmentBlueprintLibrary.generated.h"

class UPREquipmentManagerComponent;
struct FGameplayTag;
class UPREquipActionData;
struct FSpawnedVisualEntry;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPREquipmentBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static UPREquipmentManagerComponent* GetEquipmentManager(AActor* Owner);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void SyncEquipmentManager(AActor* From, AActor* To);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static bool TryEquipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void OverrideEquipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void UnequipAction(AActor* Target, UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void UnequipSlot(AActor* Target, FGameplayTag& SlotTag);
	
	// EquipmentInstance의 모든 Attached Actions를 Roguelite RunState에서 제거
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static void RemoveEquipmentInstance(AActor* Target, FGameplayTag SlotTag);

	// 두 SpawnedVisual 맵을 비교하여 MeshSpawnInfo가 변경된 ActionData들을 반환
	static TArray<UPREquipActionData*> GetChangedMeshSpawnInfoActions(
		const TMap<UPREquipActionData*, FSpawnedVisualEntry>& OldMap,
		const TMap<UPREquipActionData*, FSpawnedVisualEntry>& NewMap);
};
