// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RogueliteTypes.h"
#include "PRBlueprintFunctionLibrary.generated.h"

class APRFieldDropActor;
class UPRGameInstance;

/**
 *
 */
UCLASS()
class PROJECTREBOOT_API UPRBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*~ Mesh Utils ~*/

	UFUNCTION(BlueprintCallable, Category = "Mesh Utils")
	static TArray<UMaterialInstanceDynamic*> GetAllDynamicMaterials(UMeshComponent* TargetMesh, bool bCreateIfMissing = true);

	/*~ FieldDrop ~*/

	// 지정 위치에 확률 판정 후 쿼리 결과로 필드 드랍 액터 1개 스폰 (현재 RoomController에 등록)
	UFUNCTION(BlueprintCallable, Category = "FieldDrop", meta = (WorldContext = "WorldContextObject"))
	static APRFieldDropActor* TrySpawnFieldDrop(
		const UObject* WorldContextObject,
		const FVector& SpawnLocation,
		const FRogueliteQuery& DropQuery,
		TSubclassOf<APRFieldDropActor> DropActorClass,
		float DropChance = 1.0f);

	/*~ BGM ~*/

	// BGM 재생 (이미 같은 사운드면 무시)
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	static void PlayBGM(const UObject* WorldContextObject, USoundBase* Sound, float FadeInDuration = 0.5f);

	// BGM 정지
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	static void StopBGM(const UObject* WorldContextObject, float FadeOutDuration = 0.5f);

	// BGM 일시정지/재개
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	static void SetBGMPaused(const UObject* WorldContextObject, bool bPaused);

	// MetaSound float 파라미터 세팅
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	static void SetBGMFloatParameter(const UObject* WorldContextObject, FName ParameterName, float Value);

	// 현재 BGM 재생 중인지
	UFUNCTION(BlueprintPure, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	static bool IsBGMPlaying(const UObject* WorldContextObject);

private:
	// PRGameInstance 조회 헬퍼
	static UPRGameInstance* GetPRGameInstance(const UObject* WorldContextObject);
};
