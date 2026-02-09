// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PRRoomTypes.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PRStageConfigData.generated.h"

class UPRThemeData;

/**
 * 스테이지 설정 데이터 에셋
 * 슬롯 시퀀스, 템플릿 풀, 보상 카테고리, 전이 규칙 등 정의
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UPRStageConfigData : public UPrimaryDataAsset
	, public IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	/*~ UPrimaryDataAsset Interface ~*/
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 에셋 목록 수집
	virtual void GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const override;

	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override;

public:
	// 스테이지 표시 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	// 스테이지 인덱스 (0부터 시작)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	int32 StageIndex = 0;

	// 테마 데이터 에셋 (적/함정/장애물 풀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UPRThemeData> ThemeData;

	// 스텝 시퀀스 (스테이지 진행 순서)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	TArray<FPRRoomStep> Steps;

	// 시작 방 템플릿 (스테이지 진입 시 첫 방)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Templates")
	TSoftObjectPtr<UWorld> StartRoomTemplate;

	// 보스 맵 템플릿 (스테이지의 마지막 방)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Templates")
	TSoftObjectPtr<UWorld> BossMap;

	// 방 타입별 템플릿 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Templates")
	TMap<EPRRoomType, FPRRoomTemplatePool> Templates;

	// 방 타입별 등장 보상 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards")
	TMap<EPRRoomType, FPRRewardCategoryPool> RewardsByType;

	// 방 타입별 전이 규칙 (다음 스텝 가중치 수정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transitions")
	TMap<EPRRoomType, FPRRoomTypeTransition> TypeTransitions;

	// 방 타입별 스폰 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TMap<EPRRoomType, FPRRoomSpawnConfig> SpawnConfigs;

	// 방 타입별 흐름 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flow")
	TMap<EPRRoomType, FPRRoomFlowConfig> FlowConfigs;
};
