// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "PRGameplayGameMode.generated.h"

class APlayerCameraManager;
class APRRoomController;

// 프리웜 페이즈 델리게이트 (동기적 실행, 바인딩된 모든 핸들러 완료 후 자동 해제)
DECLARE_MULTICAST_DELEGATE(FOnRoomPrewarmSignature);

struct FPRRoomNodeInfo;
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

	// 페이드 아웃 후 방 이동 시작
	void StartFadeOutAndMoveToRoom(int32 TargetRoomIndex);

	// 페이드 아웃 완료 시 호출
	void OnFadeOutComplete();

	// 방 진입 완료 후 프리웜 시작
	void StartPrewarmPhase();

	// 프리웜 완료 후 페이드 인
	void OnPrewarmComplete();

	// 페이드 인 실행
	void ExecuteFadeIn();
	
private:
	void OnNextRoomReady(APRRoomController* RoomController, const FPRRoomNodeInfo& InNodeInfo);
	
public:
	// TEMP: 테스트용 Config (NodeInfo에 Config가 없을 때 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PR|Rules")
	FPRRoomFlowConfig DefaultRoomConfig;

	// 스테이지 설정 에셋 타입 (기본값: "StageConfig")
	UPROPERTY(EditDefaultsOnly, Category = "PR|Stage")
	FPrimaryAssetType StageConfigAssetType;

	// 프리웜 페이즈 델리게이트 (외부 시스템이 바인딩하여 프리웜 작업 수행)
	FOnRoomPrewarmSignature OnRoomPrewarm;

private:
	// 현재 방 노드 정보
	FPRRoomNodeInfo CurrentNodeInfo;

	// 페이드 대기중인 대상 방 인덱스
	int32 PendingRoomIndex = INDEX_NONE;

	// 스테이지 첫 진입 여부 (로딩 스크린용)
	bool bIsFirstRoomEntry = true;

	// 페이드 타이머 핸들
	FTimerHandle FadeTimerHandle;

	// 프리웜 대기중인 RoomController
	UPROPERTY()
	TObjectPtr<APRRoomController> PendingRoomController;

public:
	// 페이드 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "PR|Transition")
	float FadeDuration = 0.4f;

	// 프리웜 최소 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "PR|Transition")
	float MinPrewarmDuration = 0.5f;
};
