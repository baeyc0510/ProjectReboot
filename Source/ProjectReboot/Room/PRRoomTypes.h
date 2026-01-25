// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "PRRoomTypes.generated.h"

struct FPRRoomConfig;
class UStateTree;
class APRRoomController;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoomReadySignature, APRRoomController* /**RoomController*/, const FPRRoomConfig& /** Config */);

USTRUCT(BlueprintType)
struct FPRRoomConfig
{
	GENERATED_BODY()
	
	// 클리어 조건 이벤트 목표치 (태그 -> 목표 수치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, int32> TargetEventCounts;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStateTree> StateTree;
};

USTRUCT(BlueprintType)
struct FRoomInstanceInfo
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RoomIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPRRoomConfig Config; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> NextRoomIndices; // 연결된 다음 방 인덱스들

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Depth = 0; // 층수

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APRRoomController> Controller = nullptr;
};