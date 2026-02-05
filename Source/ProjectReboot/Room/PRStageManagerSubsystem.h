// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRRoomTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PRStageManagerSubsystem.generated.h"

class UPRStageConfigData;
class URogueliteSubsystem;

/*~ Delegates ~*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPRStageChangedSignature, int32, NewStageIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPRRoomGraphBuiltSignature, int32, StageIndex, const TArray<int32>&, StartRoomIndices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPRRoomEnteredSignature, int32, RoomIndex);

/**
 * 스테이지/맵 진행 관리 서브시스템
 * RogueliteSubsystem과 연동하여 스테이지 진행 담당
 */
UCLASS()
class PROJECTREBOOT_API UPRStageManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/*~ Static Access ~*/

	// 서브시스템 인스턴스 획득
	UFUNCTION(BlueprintCallable, Category = "Stage", meta = (WorldContext = "WorldContextObject"))
	static UPRStageManagerSubsystem* Get(const UObject* WorldContextObject);

	/*~ Stage Config ~*/

	// 스테이지 설정 등록
	UFUNCTION(BlueprintCallable, Category = "Stage|Config")
	void RegisterStageConfig(UPRStageConfigData* StageConfig);

	// 스테이지 설정 일괄 등록
	UFUNCTION(BlueprintCallable, Category = "Stage|Config")
	void RegisterStageConfigs(const TArray<UPRStageConfigData*>& InStageConfigs);

	// Primary Asset으로부터 스테이지 설정 로드 및 등록
	UFUNCTION(BlueprintCallable, Category = "Stage|Config")
	void LoadAndRegisterStageConfigs(FPrimaryAssetType AssetType);

	// 스테이지 설정 조회
	UFUNCTION(BlueprintPure, Category = "Stage|Config")
	UPRStageConfigData* GetStageConfig(int32 StageIndex) const;

	// 현재 스테이지 설정 조회
	UFUNCTION(BlueprintPure, Category = "Stage|Config")
	UPRStageConfigData* GetCurrentStageConfig() const;

	// 등록된 스테이지 수
	UFUNCTION(BlueprintPure, Category = "Stage|Config")
	int32 GetStageCount() const { return StageConfigs.Num(); }

	/*~ Progress State ~*/

	// 스테이지 진행 상태 조회
	UFUNCTION(BlueprintPure, Category = "Stage|Progress")
	const FPRStageProgress& GetStageProgress() const { return StageProgress; }

	// 현재 스테이지 인덱스
	UFUNCTION(BlueprintPure, Category = "Stage|Progress")
	int32 GetCurrentStageIndex() const { return StageProgress.CurrentStageIndex; }

	// 현재 방 인덱스
	UFUNCTION(BlueprintPure, Category = "Stage|Progress")
	int32 GetCurrentRoomIndex() const { return StageProgress.CurrentRoomIndex; }

	/*~ Room Graph ~*/

	// 방 노드 정보 조회
	const FPRRoomNodeInfo* GetRoomNodeInfo(int32 RoomIndex) const;

	// 현재 방 노드 정보 조회
	const FPRRoomNodeInfo* GetCurrentRoomNodeInfo() const;

	// 다음 방 인덱스 목록 조회
	TArray<int32> GetNextRoomIndices(int32 RoomIndex) const;

	// 시작 방 인덱스 목록 조회
	const TArray<int32>& GetStartRoomIndices() const { return StartRoomIndices; }

	/*~ Room Transition ~*/

	// 인덱스 기반 방 진입
	UFUNCTION(BlueprintCallable, Category = "Stage|Room")
	void EnterRoomByIndex(int32 RoomIndex);

	// 방 클리어 (다음 선택지 준비)
	UFUNCTION(BlueprintCallable, Category = "Stage|Room")
	void OnRoomCleared();

	// 보스 처치 (다음 스테이지 또는 런 완료)
	UFUNCTION(BlueprintCallable, Category = "Stage|Room")
	void OnBossDefeated();

	/*~ Save/Load (Seamless Travel) ~*/

	// 스테이지 진행 상태 저장
	UFUNCTION(BlueprintCallable, Category = "Stage|Save")
	void SaveStageProgress();

	// 스테이지 진행 상태 로드
	UFUNCTION(BlueprintCallable, Category = "Stage|Save")
	void LoadStageProgress();

public:
	/*~ Delegates ~*/

	// 스테이지 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FPRStageChangedSignature OnStageChanged;

	// 방 그래프 빌드 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FPRRoomGraphBuiltSignature OnRoomGraphBuilt;

	// 방 진입 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FPRRoomEnteredSignature OnRoomEntered;

protected:
	/*~ RogueliteSubsystem 연동 ~*/

	// 런 시작 시 호출 (RogueliteSubsystem.OnRunStarted 연동)
	UFUNCTION()
	void HandleRunStarted();

	// 런 종료 시 호출 (RogueliteSubsystem.OnRunEnded 연동)
	UFUNCTION()
	void HandleRunEnded(bool bCompleted);

	/*~ Internal ~*/

	// 스테이지 방 그래프 생성
	void BuildRoomGraphForStage(int32 StageIndex);

	// 슬롯별 방 노드 생성
	TArray<FPRRoomNodeInfo> BuildSlotNodes(int32 SlotIndex, EPRRoomType LastSelectedRoomType, int32& OutNextRoomIndex) const;

	// 결정적 랜덤 스트림 생성
	FRandomStream CreateDeterministicRandom(int32 StageIndex, int32 SlotIndex, EPRRoomType LastSelectedRoomType) const;

	// 가중치 수정 적용 (이전 선택 타입 기준)
	TMap<EPRRoomType, float> GetModifiedWeightsForLastType(int32 SlotIndex, EPRRoomType LastSelectedRoomType) const;

	// 가중치 기반 랜덤 선택
	EPRRoomType SelectWeightedRandom(const TMap<EPRRoomType, float>& Weights, FRandomStream& Random) const;

	// 보상 카테고리 선택 (ChoiceCount만큼, 중복 없이)
	TArray<FPRRewardCategoryEntry> SelectRewardCategories(const FPRRewardCategoryPool& Pool, int32 Count, FRandomStream& Random) const;

	// 템플릿 선택
	TSoftObjectPtr<UWorld> SelectTemplate(EPRRoomType RoomType, FRandomStream& Random) const;

private:
	/*~ Stage Configs ~*/

	// 등록된 스테이지 설정
	UPROPERTY()
	TMap<int32, TObjectPtr<UPRStageConfigData>> StageConfigs;

	/*~ Stage Progress ~*/

	// 현재 스테이지 진행 상태
	UPROPERTY()
	FPRStageProgress StageProgress;

	// 저장된 스테이지 진행 상태 (Seamless Travel용)
	UPROPERTY()
	FPRStageProgress SavedStageProgress;

	// 랜덤 스트림
	FRandomStream RandomStream;

	/*~ Room Graph ~*/

	// 방 노드 그래프 (RoomIndex -> NodeInfo)
	UPROPERTY()
	TMap<int32, FPRRoomNodeInfo> RoomGraph;

	// 시작 방 인덱스 목록
	UPROPERTY()
	TArray<int32> StartRoomIndices;

	// 그래프가 생성된 스테이지 인덱스
	int32 GraphStageIndex = INDEX_NONE;
};
