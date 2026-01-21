// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PRRoomTypes.generated.h"

struct FPRRoomConfig;
class UStateTree;
class APRRoomController;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoomReadySignature, APRRoomController* /**RoomController*/, const FPRRoomConfig& /** Config */);

USTRUCT(BlueprintType)
struct FPRRoomConfig
{
	GENERATED_BODY()
	
	// TODO: 추후 Wave 기반으로 변경
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetKillCount = 1;
	
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