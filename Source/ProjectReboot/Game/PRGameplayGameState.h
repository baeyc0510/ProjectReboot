// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "PRGameplayGameState.generated.h"

struct FGameplayTag;
class APRRoomController;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API APRGameplayGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Room")
	APRRoomController* GetCurrentRoomController() const {return CurrentRoomController;}
	
	UFUNCTION(BlueprintCallable, Category = "Room")
	void NotifyRoomEnter(APRRoomController* NewRoom);
	
	// 이벤트를 현재 방의 StateTree로 전달
	UFUNCTION(BlueprintCallable, Category = "Room")
	void SendRoomEvent(const FGameplayTag& EventTag);

	// 이벤트 카운트 증가
	UFUNCTION(BlueprintCallable, Category = "Room")
	void AddEventCount(const FGameplayTag& EventTag, int32 Delta = 1);

	// 이벤트 카운트 조회
	UFUNCTION(BlueprintPure, Category = "Room")
	int32 GetEventCount(const FGameplayTag& EventTag) const;

public:
	// 현재 플레이어가 있는 방
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	APRRoomController* CurrentRoomController;
	
	// 현재 이벤트 카운트 (태그 -> 누적 수치)
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	TMap<FGameplayTag, int32> CurrentEventCounts;
};