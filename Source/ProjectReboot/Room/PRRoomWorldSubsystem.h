// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRRoomTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PRRoomWorldSubsystem.generated.h"

class APRRoomController;
class ULevelStreamingDynamic;

/**
 * 월드별 방 컨트롤러 관리 서브시스템
 * 레벨 전환 시 자동으로 정리됨
 */
UCLASS()
class PROJECTREBOOT_API UPRRoomWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/*~ Level Instance ~*/

	// 방 템플릿 레벨 인스턴스 로드
	UFUNCTION(BlueprintCallable, Category = "Room|Level")
	void LoadRoomTemplate(int32 RoomIndex, TSoftObjectPtr<UWorld> Template, const FVector& Location = FVector::ZeroVector);

	// 방 레벨 인스턴스 언로드
	UFUNCTION(BlueprintCallable, Category = "Room|Level")
	void UnloadRoom(int32 RoomIndex);

	// 모든 방 레벨 인스턴스 언로드
	UFUNCTION(BlueprintCallable, Category = "Room|Level")
	void UnloadAllRooms();

	/*~ Room Controller ~*/

	// 방 컨트롤러 등록 및 초기화 대기
	void CallOrRegister_RoomReady(int32 RoomIndex, FOnRoomReadySignature::FDelegate&& OnReadyCallback);

	// 방 컨트롤러 등록
	void RegisterRoomController(int32 RoomIndex, APRRoomController* InController);

	// 방 컨트롤러 조회
	UFUNCTION(BlueprintPure, Category = "Room")
	APRRoomController* GetRoomController(int32 RoomIndex) const;

	// 현재 활성 방 컨트롤러
	UFUNCTION(BlueprintPure, Category = "Room")
	APRRoomController* GetCurrentRoomController() const { return CurrentRoomController; }

	// 현재 활성 방 설정
	void SetCurrentRoom(int32 RoomIndex);

	/*~ Player Teleport ~*/

	// 플레이어를 지정된 방으로 텔레포트
	UFUNCTION(BlueprintCallable, Category = "Room|Teleport")
	void TeleportPlayerToRoom(int32 RoomIndex, APlayerController* PlayerController = nullptr);

	// 모든 플레이어를 지정된 방으로 텔레포트
	UFUNCTION(BlueprintCallable, Category = "Room|Teleport")
	void TeleportAllPlayersToRoom(int32 RoomIndex);

protected:
	// 레벨 로드 완료 콜백
	UFUNCTION()
	void OnLevelInstanceLoaded();

private:
	// 현재 월드의 방 정보
	UPROPERTY()
	TMap<int32, FRoomInstanceInfo> RoomMap;

	// 방 준비 대기 콜백
	TMap<int32, FOnRoomReadySignature> PendingRoomReadyRequests;

	// 로드된 레벨 인스턴스
	UPROPERTY()
	TMap<int32, TObjectPtr<ULevelStreamingDynamic>> LoadedLevelInstances;

	// 로드 중인 방 인덱스 (콜백용)
	int32 PendingLoadRoomIndex = -1;

	// 현재 활성 방 컨트롤러
	UPROPERTY()
	TObjectPtr<APRRoomController> CurrentRoomController;

	// 현재 방 인덱스
	int32 CurrentRoomIndex = -1;
};
