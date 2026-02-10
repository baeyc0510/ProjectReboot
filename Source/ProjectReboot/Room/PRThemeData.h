// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PRThemeData.generated.h"

class APREnemyCharacter;

/**
 * 적 스폰 항목
 */
USTRUCT(BlueprintType)
struct FPREnemySpawnEntry
{
	GENERATED_BODY()

	// 스폰할 적 캐릭터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APREnemyCharacter> EnemyClass;

	// 스폰 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	// 최소 난이도 배율 (이 값 이상에서만 등장)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MinDifficulty = 0.0f;

	// 최대 난이도 배율 (이 값 이하에서만 등장, 0 = 무제한)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MaxDifficulty = 0.0f;
};

/**
 * 환경 오브젝트(함정/장애물) 스폰 항목
 */
USTRUCT(BlueprintType)
struct FPREnvironmentSpawnEntry
{
	GENERATED_BODY()

	// 스폰할 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;

	// 스폰 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

/**
 * 테마 데이터 에셋
 * 스테이지별 적, 함정, 장애물 풀 정의
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UPRThemeData : public UDataAsset, public IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override;

public:
	/*~ Info ~*/

	// 테마 표시 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	/*~ Enemies ~*/

	// 일반 적 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies")
	TArray<FPREnemySpawnEntry> NormalEnemies;

	// 엘리트 적 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies")
	TArray<FPREnemySpawnEntry> EliteEnemies;

	// 미니보스 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies")
	TArray<FPREnemySpawnEntry> MiniBosses;

	/*~ Environment ~*/

	// 함정 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment")
	TArray<FPREnvironmentSpawnEntry> Traps;

	// 장애물 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Environment")
	TArray<FPREnvironmentSpawnEntry> Obstacles;

};
