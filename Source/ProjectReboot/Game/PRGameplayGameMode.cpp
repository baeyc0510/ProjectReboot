// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameMode.h"
#include "PRGameplayGameState.h"
#include "StateTree.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomWorldSubsystem.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRStageManagerSubsystem.h"
#include "RogueliteSubsystem.h"

void APRGameplayGameMode::OnGameplayEvent(const FGameplayTag& EventTag, int32 Delta)
{
	APRGameplayGameState* GS = GetGameState<APRGameplayGameState>();
	if (!GS)
	{
		return;
	}

	GS->AddEventCount(EventTag, Delta);

	// StateTree에서 클리어 조건을 판정하므로 여기서는 카운트만 증가
}

void APRGameplayGameMode::MoveToNextRoom(int32 TargetRoomIndex)
{
	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		// 방 준비 완료 시 OnNextRoomReady 호출 등록
		RoomSubsystem->CallOrRegister_RoomReady(TargetRoomIndex,
			FOnRoomReadySignature::FDelegate::CreateUObject(this, &APRGameplayGameMode::OnNextRoomReady));
	}

	// 방 진입 시작 (레벨 로드 또는 보스 맵 전환)
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->EnterRoomByIndex(TargetRoomIndex);
	}
}

void APRGameplayGameMode::BeginPlay()
{
	// 스테이지 설정 로드 및 등록
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		// AssetType이 설정되지 않았으면 기본값 사용
		FPrimaryAssetType AssetType = StageConfigAssetType.IsValid() ? StageConfigAssetType : FPrimaryAssetType(TEXT("PRStageConfig"));
		StageManager->LoadAndRegisterStageConfigs(AssetType);

		// 스테이지 진행 로드 (Seamless Travel 후 복구)
		StageManager->LoadStageProgress();
	}
	
	Super::BeginPlay();

	// 런 시작 (RoomGraph 생성)
	if (URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this))
	{
		if (!RogueliteSubsystem->IsRunActive())
		{
			RogueliteSubsystem->StartRun();	
		}
	}
	
	MoveToNextRoom (0);
}

void APRGameplayGameMode::OnRoomCleared()
{
	// 방 클리어 알림
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->SendRoomEvent(TAG_Event_Room_Clear);
	}
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->OnRoomCleared();
	}

	UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room cleared!"));
}

void APRGameplayGameMode::OnNextRoomReady(APRRoomController* RoomController, const FPRRoomNodeInfo& InNodeInfo)
{
	CurrentNodeInfo = InNodeInfo;

	// NodeInfo의 Config가 비어있으면 DefaultRoomConfig 사용
	if (!IsValid(CurrentNodeInfo.FlowConfig.StateTree) && IsValid(DefaultRoomConfig.StateTree))
	{
		CurrentNodeInfo.FlowConfig.StateTree = DefaultRoomConfig.StateTree;
	}

	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->NotifyRoomEnter(RoomController);
	}

	if (IsValid(RoomController))
	{
		// 플레이어를 해당 방으로 텔레포트
		if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
		{
			RoomSubsystem->TeleportAllPlayersToRoom(RoomController->GetRoomIndex());
		}
		
		RoomController->InitRoom(CurrentNodeInfo);
		RoomController->ActivateRoom();

		UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room %d ready and activated"), RoomController->GetRoomIndex());
	}
}
