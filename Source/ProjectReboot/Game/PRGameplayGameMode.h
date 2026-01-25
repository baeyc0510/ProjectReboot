// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "PRGameplayGameMode.generated.h"

struct FPRRoomConfig;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API APRGameplayGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// 이벤트 카운트 증가 및 클리어 판정
	UFUNCTION(BlueprintCallable, Category = "PR|Rules")
	void OnGameplayEvent(const FGameplayTag& EventTag, int32 Delta = 1);

	// 다음 방으로의 전환을 시작 (포탈 상호작용 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "PR|Flow")
	void MoveToNextRoom(int32 TargetRoomIndex);

protected:
	virtual void BeginPlay() override;

	virtual void OnRoomCleared();
	
private:
	void OnNextRoomReady( APRRoomController* RoomController, const FPRRoomConfig& RoomConfig);
	
public:
	// TEMP: 테스트용 Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PR|Rules")
	FPRRoomConfig DefaultRoomConfig;
	
private:
	FPRRoomConfig CurrentRoomConfig;
};
