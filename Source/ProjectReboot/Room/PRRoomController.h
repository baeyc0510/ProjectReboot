// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRRoomTypes.h"
#include "GameFramework/Actor.h"
#include "PRRoomController.generated.h"

class UPRRoomStateTreeComponent;
class APRRoomDoor;

UCLASS()
class PROJECTREBOOT_API APRRoomController : public AActor
{
	GENERATED_BODY()

public:
	APRRoomController();
	
	// StateTree 컴포넌트 반환
	UFUNCTION(BlueprintPure, Category = "Room")
	UPRRoomStateTreeComponent* GetStateTreeComponent() const {return StateTreeComponent;}
	
	// 방 설정 초기화 (NodeInfo의 Config 사용)
	void InitRoom(const FPRRoomNodeInfo& InNodeInfo);
	
	// 방 활성화
	void ActivateRoom();
	
	// 방 인덱스 반환
	int32 GetRoomIndex() const;
	
	// 문 상호작용 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SetDoorsInteractable(bool bEnabled);

	// 문 표시 및 활성화
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void ShowDoors();

	// 문 숨김 및 비활성화
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void HideAndDisableDoors();

	// 문 동적 스폰 및 다음 방 정보 할당
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SpawnDoors(const TArray<int32>& NextRoomIndices);

	// 스폰된 문 전체 파괴
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void DestroyDoors();

	// 방 노드 정보 조회
	UFUNCTION(BlueprintPure, Category = "Room")
	const FPRRoomNodeInfo& GetNodeInfo() const { return NodeInfo; }

	// 스폰 정보 조회
	UFUNCTION(BlueprintPure, Category = "Room|Spawn")
	const FPRRoomSpawnInfo& GetSpawnInfo() const { return NodeInfo.SpawnInfo; }

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 레벨 이름에서 방 인덱스 추출
	int32 ExtractRoomIndexFromLevelName() const;
	
public:
	// 문 스폰 중앙 위치 (nullptr이면 RoomController 위치 사용)
	UPROPERTY(EditAnywhere, Category = "Room|Door")
	TObjectPtr<AActor> DoorSpawnPoint;

	// 문 액터 클래스
	UPROPERTY(EditAnywhere, Category = "Room|Door")
	TSubclassOf<APRRoomDoor> DoorClass;

	// 문 간 간격 (중앙 기준 좌우 배치)
	UPROPERTY(EditAnywhere, Category = "Room|Door")
	float DoorSpacing = 300.f;

	UPROPERTY(EditAnywhere, Category = "Room|Door")
	TObjectPtr<USoundBase> DoorActivationSound;

	// 플레이어 스폰 위치 (nullptr이면 RoomController 위치 사용)
	UPROPERTY(EditAnywhere, Category = "Room")
	TObjectPtr<AActor> PlayerSpawnPoint;

	// 플레이어 스폰 위치 반환
	UFUNCTION(BlueprintPure, Category = "Room")
	FTransform GetPlayerSpawnTransform() const;

	// 적 스폰 포인트 배열
	UPROPERTY(EditAnywhere, Category = "Room|Spawn")
	TArray<AActor*> EnemySpawnPoints;

	// 보상 스폰 위치 (nullptr이면 RoomController 위치 사용)
	UPROPERTY(EditAnywhere, Category = "Room|Reward")
	TObjectPtr<AActor> RewardSpawnPoint;

	// 보상 액터 스폰 및 설정
	UFUNCTION(BlueprintCallable, Category = "Room|Reward")
	AActor* SpawnReward(TSubclassOf<AActor> RewardActorClass);

	// 필드 드랍 스폰 포인트 배열
	UPROPERTY(EditAnywhere, Category = "Room|FieldDrop")
	TArray<AActor*> FieldDropSpawnPoints;

	/*~ Spawned Actor Management ~*/

	// 동적 스폰 액터 등록 (드랍, 보상, 적 등)
	UFUNCTION(BlueprintCallable, Category = "Room")
	void RegisterSpawnedActor(AActor* Actor);

	// 등록된 모든 동적 스폰 액터 파괴
	UFUNCTION(BlueprintCallable, Category = "Room")
	void DestroyAllSpawnedActors();

	/*~ Wave Management ~*/

	// 현재 웨이브 인덱스 반환
	UFUNCTION(BlueprintPure, Category = "Room|Wave")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	// 총 웨이브 수 반환
	UFUNCTION(BlueprintPure, Category = "Room|Wave")
	int32 GetTotalWaveCount() const { return TotalWaveCount; }

	// 다음 웨이브로 증가 (마지막 웨이브면 false 반환)
	UFUNCTION(BlueprintCallable, Category = "Room|Wave")
	bool IncrementWaveIndex();

	// 웨이브 인덱스 초기화
	UFUNCTION(BlueprintCallable, Category = "Room|Wave")
	void ResetWaveIndex() { CurrentWaveIndex = 0; }

	// 현재 방 인덱스 (Level Instance 기반)
	UPROPERTY(VisibleAnywhere, Category = "Room")
	int32 RoomIndex = -1;

	// 현재 웨이브 인덱스
	UPROPERTY(VisibleAnywhere, Category = "Room|Wave")
	int32 CurrentWaveIndex = 0;

	// 총 웨이브 수 (InitRoom에서 설정)
	UPROPERTY(VisibleAnywhere, Category = "Room|Wave")
	int32 TotalWaveCount = 0;

	// 방 노드 정보 (런 시작 시 확정, StageManager에서 복사)
	UPROPERTY(VisibleAnywhere, Category = "Room")
	FPRRoomNodeInfo NodeInfo;

	// 동적 스폰된 문 목록
	UPROPERTY(VisibleAnywhere, Transient, Category = "Room|Door")
	TArray<TObjectPtr<APRRoomDoor>> Doors;

	// 동적 스폰 액터 목록 (방 전환 시 일괄 파괴)
	UPROPERTY(VisibleAnywhere, Category = "Room")
	TArray<TObjectPtr<AActor>> SpawnedActors;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPRRoomStateTreeComponent> StateTreeComponent;

	// 웨이브 클리어 이벤트 핸들러
	void HandleWaveClearEvent(const FGameplayTag& EventTag, int32 Count);

	// 웨이브 클리어 델리게이트 핸들
	FDelegateHandle WaveClearHandle;
};