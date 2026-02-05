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

	// 입구 문 상호작용 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SetEntryDoorsInteractable(bool bEnabled);

	// 출구 문 상호작용 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SetExitDoorsInteractable(bool bEnabled);

	// 출구 문에 다음 방 정보 할당
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void AssignExitDoors(const TArray<int32>& NextRoomIndices);
	
protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

	// 레벨 이름에서 방 인덱스 추출
	int32 ExtractRoomIndexFromLevelName() const;
	
public:
	// 방 안에 배치된 문(Door) 액터들, TODO: Interface 활용한 Open / Close 처리
	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<AActor*> EntryDoors;

	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<AActor*> ExitDoors;

	// 플레이어 스폰 위치 (nullptr이면 RoomController 위치 사용)
	UPROPERTY(EditAnywhere, Category = "Room")
	TObjectPtr<AActor> PlayerSpawnPoint;

	// 플레이어 스폰 위치 반환
	UFUNCTION(BlueprintPure, Category = "Room")
	FTransform GetPlayerSpawnTransform() const;
	
	// 현재 방 인덱스 (Level Instance 기반)
	UPROPERTY(VisibleAnywhere, Category = "Room")
	int32 RoomIndex = -1;

	// 방 노드 정보 (런 시작 시 확정, StageManager에서 복사)
	UPROPERTY(VisibleAnywhere, Category = "Room")
	FPRRoomNodeInfo NodeInfo;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPRRoomStateTreeComponent> StateTreeComponent;
};