// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameMode.h"
#include "PRGameplayGameState.h"
#include "StateTree.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomWorldSubsystem.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRStageManagerSubsystem.h"

void APRGameplayGameMode::OnGameplayEvent(const FGameplayTag& EventTag, int32 Delta)
{
	APRGameplayGameState* GS = GetGameState<APRGameplayGameState>();
	if (!GS)
	{
		return;
	}

	GS->AddEventCount(EventTag, Delta);

	// 클리어 조건 판정: 모든 목표 이벤트 수치 달성 여부
	const FPRRoomFlowConfig& RoomConfig = CurrentNodeInfo.FlowConfig;
	bool bAllGoalsMet = true;
	if (RoomConfig.TargetEventCounts.Num() <= 0)
	{
		bAllGoalsMet = false;
	}
	else
	{
		for (const TPair<FGameplayTag, int32>& Goal : RoomConfig.TargetEventCounts)
		{
			const int32 CurrentCount = GS->GetEventCount(Goal.Key);
			if (CurrentCount < Goal.Value)
			{
				bAllGoalsMet = false;
				break;
			}
		}
	}

	if (bAllGoalsMet)
	{
		OnRoomCleared();
	}
}

void APRGameplayGameMode::MoveToNextRoom(int32 TargetRoomIndex)
{
	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		RoomSubsystem->CallOrRegister_RoomReady(TargetRoomIndex,
			FOnRoomReadySignature::FDelegate::CreateUObject(this, &APRGameplayGameMode::OnNextRoomReady));
	}
}

void APRGameplayGameMode::BeginPlay()
{
	Super::BeginPlay();

	// PRStageManagerSubsystem에서 스테이지 진행 로드 (Seamless Travel 후 복구)
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->LoadStageProgress();
	}

	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		// 첫 방 (RoomIndex 0)으로 이동
		MoveToNextRoom(0);
	}
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
	if (CurrentNodeInfo.FlowConfig.TargetEventCounts.Num() <= 0 && DefaultRoomConfig.TargetEventCounts.Num() > 0)
	{
		CurrentNodeInfo.FlowConfig.TargetEventCounts = DefaultRoomConfig.TargetEventCounts;
	}

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
		RoomController->InitRoom(CurrentNodeInfo);
		RoomController->ActivateRoom();

		// 플레이어를 해당 방으로 텔레포트
		if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
		{
			RoomSubsystem->TeleportAllPlayersToRoom(RoomController->GetRoomIndex());
		}

		UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room %d ready and activated"), RoomController->GetRoomIndex());
	}
}
