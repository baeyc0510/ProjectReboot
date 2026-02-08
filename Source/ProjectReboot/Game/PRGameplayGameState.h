// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "PRGameplayGameState.generated.h"

struct FGameplayTag;
class APRRoomController;

/*~ Delegates ~*/

// 이벤트 카운트 변경 시 호출
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEventCountChangedSignature, const FGameplayTag& /*EventTag*/, int32 /*NewCount*/);

/**
 *
 */
UCLASS()
class PROJECTREBOOT_API APRGameplayGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/*~ Room ~*/

	// 현재 방 컨트롤러 조회
	UFUNCTION(BlueprintPure, Category = "Room")
	APRRoomController* GetCurrentRoomController() const { return CurrentRoomController; }

	// 방 진입 알림
	UFUNCTION(BlueprintCallable, Category = "Room")
	void NotifyRoomEnter(APRRoomController* NewRoom);

	// 이벤트를 현재 방의 StateTree로 전달
	UFUNCTION(BlueprintCallable, Category = "Room")
	void SendRoomEvent(const FGameplayTag& EventTag);

	/*~ Event Count ~*/

	// 이벤트 카운트 증가
	UFUNCTION(BlueprintCallable, Category = "Room")
	void AddEventCount(const FGameplayTag& EventTag, int32 Delta = 1);

	// 이벤트 카운트 조회
	UFUNCTION(BlueprintPure, Category = "Room")
	int32 GetEventCount(const FGameplayTag& EventTag) const;

	// 이벤트 카운트 리셋
	UFUNCTION(BlueprintCallable, Category = "Room")
	void ResetEventCount(const FGameplayTag& EventTag);

	/*~ Delegates ~*/

	// 이벤트 카운트 변경 델리게이트
	FOnEventCountChangedSignature OnEventCountChanged;

private:
	// 현재 플레이어가 있는 방
	UPROPERTY()
	TObjectPtr<APRRoomController> CurrentRoomController;

	// 현재 이벤트 카운트 (태그 -> 누적 수치)
	UPROPERTY()
	TMap<FGameplayTag, int32> CurrentEventCounts;
};
