// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PRRoomTypes.h"
#include "PRStageConfigData.generated.h"

class UPRThemeData;

/**
 * 스테이지 설정 데이터 에셋
 * 슬롯 시퀀스, 템플릿 풀, 보상 카테고리, 전이 규칙 등 정의
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UPRStageConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/*~ UPrimaryDataAsset Interface ~*/
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	/*~ Basic Info ~*/

	// 스테이지 표시 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	// 스테이지 인덱스 (0부터 시작)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	int32 StageIndex = 0;

	/*~ Theme ~*/

	// 테마 데이터 에셋 (적/함정/장애물 풀)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UPRThemeData> ThemeData;

	/*~ Slot Sequence ~*/

	// 슬롯 시퀀스 (스테이지 진행 순서)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slots")
	TArray<FPRRoomSlot> Slots;

	/*~ Templates ~*/

	// 방 타입별 템플릿 풀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Templates")
	TMap<EPRRoomType, FPRRoomTemplatePool> Templates;

	/*~ Rewards ~*/

	// 방 타입별 보상 카테고리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards")
	TMap<EPRRoomType, FPRRewardCategoryPool> RewardsByType;

	/*~ Transitions ~*/

	// 방 타입별 전이 규칙 (다음 슬롯 가중치 수정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transitions")
	TMap<EPRRoomType, FPRRoomTypeTransition> TypeTransitions;

	/*~ Boss ~*/

	// 보스 맵 (Seamless Travel 대상)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	TSoftObjectPtr<UWorld> BossMap;
};
