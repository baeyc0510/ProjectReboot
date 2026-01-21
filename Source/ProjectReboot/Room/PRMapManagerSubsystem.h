// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRRoomTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PRMapManagerSubsystem.generated.h"

struct FRoomInstanceInfo;
/**
 * 맵 생성 및 관리 시스템
 */
UCLASS()
class PROJECTREBOOT_API UPRMapManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void GenerateRoomConfigs(); // TODO: 추후 절차적 생성 로직 구현
	void CallOrRegister_RoomReady(int32 RoomIndex, FOnRoomReadySignature::FDelegate&& OnReadyCallback);
	void RegisterRoomController(int32 RoomIndex, APRRoomController* InController);
	
private:
	UPROPERTY()
	TMap<int32, FRoomInstanceInfo> RunMap;
	
	TMap<int32, FOnRoomReadySignature> PendingRoomReadyRequests;
};