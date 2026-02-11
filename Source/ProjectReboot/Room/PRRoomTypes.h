// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PRRoomTypes.generated.h"

class UStateTree;
class APRRoomController;
class URoguelitePoolPreset;
class APREnemyCharacter;

/*~ Enums ~*/

/**
 * 방 타입 열거형
 */
UENUM(BlueprintType)
enum class EPRRoomType : uint8
{
	// 초기값 (런/스테이지 시작)
	Default,
	// 일반 전투 방
	Combat,
	// 엘리트 적 방
	Elite,
	// 상점 방
	Shop,
	// 보물 방
	Treasure,
	// 휴식 방
	Rest,
	// 보스 방
	Boss
};

/**
 * 방 흐름 설정 (StateTree)
 */
USTRUCT(BlueprintType)
struct FPRRoomFlowConfig
{
	GENERATED_BODY()

	// 방 로직 StateTree (클리어 조건은 StateTree 내부에서 관리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStateTree> StateTree;
};

/**
 * 방 스폰 설정 (적/환경 스폰 수량)
 */
USTRUCT(BlueprintType)
struct FPRRoomSpawnConfig
{
	GENERATED_BODY()

	// 최소 웨이브 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 MinWaveCount = 1;

	// 최대 웨이브 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 MaxWaveCount = 1;

	// 웨이브당 최소 일반 적 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MinNormalEnemies = 3;

	// 웨이브당 최대 일반 적 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MaxNormalEnemies = 5;

	// 웨이브당 최소 엘리트 적 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MinEliteEnemies = 0;

	// 웨이브당 최대 엘리트 적 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MaxEliteEnemies = 0;

	// 최소 미니보스 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MinMiniBosses = 0;

	// 최대 미니보스 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 MaxMiniBosses = 0;
};

/**
 * 웨이브별 스폰 정보 (확정된 정보)
 */
USTRUCT(BlueprintType)
struct FPRWaveSpawnInfo
{
	GENERATED_BODY()

	// 스폰할 적 클래스 → 수량 매핑
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<TSubclassOf<APREnemyCharacter>, int32> EnemySpawnMap;
};

/**
 * 방 스폰 정보 (확정된 정보)
 */
USTRUCT(BlueprintType)
struct FPRRoomSpawnInfo
{
	GENERATED_BODY()

	// 웨이브 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPRWaveSpawnInfo> Waves;
};

/**
 * 방 노드 정보 (런 시작 시 확정되는 정의 데이터)
 */
USTRUCT(BlueprintType)
struct FPRRoomNodeInfo
{
	GENERATED_BODY()

	// 방 인덱스 (그래프 내 고유 ID)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RoomIndex = -1;

	// 스텝 인덱스 (진행 단계)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 StepIndex = 0;

	// 방 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPRRoomType RoomType = EPRRoomType::Default;

	// 보상 카테고리 (문에 표시될 태그)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayTag RewardCategory;

	// 클리어 후 보상 풀 프리셋
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<URoguelitePoolPreset> RewardPoolPreset = nullptr;

	// 방 템플릿 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Template;

	// 방 내부 배치용 시드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Seed = 0;

	// 난이도 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Difficulty = 1.0f;

	// 연결된 다음 방 인덱스들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> NextRoomIndices;

	// 방 흐름 설정 (StateTree, 클리어 조건)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPRRoomFlowConfig FlowConfig;

	// 방 스폰 정보 (확정된 웨이브/적 정보)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPRRoomSpawnInfo SpawnInfo;
};

/**
 * 방 인스턴스 정보 (런타임, 월드 상태)
 */
USTRUCT(BlueprintType)
struct FRoomInstanceInfo
{
	GENERATED_BODY()

	// 방 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RoomIndex = -1;

	// 스텝 인덱스 (진행 단계)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 StepIndex = 0;

	// 방 컨트롤러
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APRRoomController> Controller = nullptr;
};

/*~ Reward Types ~*/

/**
 * 보상 카테고리 항목
 */
USTRUCT(BlueprintType)
struct FPRRewardCategoryEntry
{
	GENERATED_BODY()

	// 문에 표시될 카테고리 태그 (e.g., Reward.WeaponUpgrade)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Reward"))
	FGameplayTag Category;

	// 클리어 후 실제 선택지 쿼리용 프리셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<URoguelitePoolPreset> PoolPreset = nullptr;

	// 선택 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

/**
 * 방 타입별 보상 카테고리 풀
 */
USTRUCT(BlueprintType)
struct FPRRewardCategoryPool
{
	GENERATED_BODY()

	// 보상 카테고리 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPRRewardCategoryEntry> Categories;
};

/*~ Template Types ~*/

/**
 * 방 템플릿 항목
 */
USTRUCT(BlueprintType)
struct FPRRoomTemplateEntry
{
	GENERATED_BODY()

	// 방 레벨 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Level;

	// 선택 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

/**
 * 방 타입별 템플릿 풀
 */
USTRUCT(BlueprintType)
struct FPRRoomTemplatePool
{
	GENERATED_BODY()

	// 템플릿 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPRRoomTemplateEntry> Entries;
};

/*~ Transition Types ~*/

/**
 * 방 타입 전이 규칙 (다음 스텝에 적용될 가중치 수정)
 */
USTRUCT(BlueprintType)
struct FPRRoomTypeTransition
{
	GENERATED_BODY()

	// 다음 스텝의 타입별 가중치 배율
	// 0 = 등장 불가, 0.5 = 확률 절반, 2.0 = 확률 2배
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EPRRoomType, float> WeightModifiers;
};

/*~ Step Types ~*/

/**
 * 스텝 정의 (스테이지 내 각 단계별 연결 규칙)
 * 노드 수는 이전 스텝의 연결에서 자연스럽게 도출됨
 */
USTRUCT(BlueprintType)
struct FPRRoomStep
{
	GENERATED_BODY()

	// 각 노드에서 다음 스텝으로 연결할 최소 분기 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 MinBranchCount = 1;

	// 각 노드에서 다음 스텝으로 연결할 최대 분기 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 MaxBranchCount = 3;

	// 가능한 방 타입 및 가중치 (노드마다 독립 선택)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EPRRoomType, float> PossibleTypes;

	// TODO: 난이도 배율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Difficulty = 1.0f;
};

/*~ Runtime Types ~*/

/**
 * 방 선택지 (런타임 생성)
 */
USTRUCT(BlueprintType)
struct FPRRoomChoice
{
	GENERATED_BODY()

	// 선택된 방 타입
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPRRoomType RoomType = EPRRoomType::Default;

	// 사용할 템플릿 레벨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Template;

	// 보상 카테고리 (문에 아이콘으로 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayTag RewardCategory;

	// 클리어 후 실제 선택지 쿼리에 사용할 프리셋 (Shop/Boss는 nullptr)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<URoguelitePoolPreset> RewardPoolPreset = nullptr;

	// 방 내부 배치용 시드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Seed = 0;

	// 스텝의 난이도 배율
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Difficulty = 1.0f;
};

/**
 * 스테이지 진행 상태 (RogueliteSubsystem의 런과 연동)
 */
USTRUCT(BlueprintType)
struct FPRStageProgress
{
	GENERATED_BODY()

	// 시드 (재현용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MasterSeed = 0;

	// 현재 스테이지 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentStageIndex = 0;

	// 현재 방 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentRoomIndex = -1;

	// 진행 초기화
	void Reset()
	{
		MasterSeed = 0;
		CurrentStageIndex = 0;
		CurrentRoomIndex = -1;
	}
};

/*~ Delegates ~*/

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoomReadySignature, APRRoomController* /*RoomController*/, const FPRRoomNodeInfo& /*NodeInfo*/);
