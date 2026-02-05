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
	const FPRRoomConfig& RoomConfig = CurrentNodeInfo.Config;
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
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->SendRoomEvent(TAG_Event_Room_Clear);
	}

	// PRStageManagerSubsystem에 방 클리어 알림
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->OnRoomCleared();

		if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
		{
			if (APRRoomController* RoomController = GS->GetCurrentRoomController())
			{
				// 현재 방의 NextRoomIndices를 가져와 문에 할당
				const TArray<int32> NextRoomIndices = CurrentNodeInfo.NextRoomIndices;
				RoomController->AssignExitDoors(NextRoomIndices);
				RoomController->SetExitDoorsInteractable(true);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room cleared!"));
}

void APRGameplayGameMode::OnNextRoomReady(APRRoomController* RoomController, const FPRRoomNodeInfo& InNodeInfo)
{
	CurrentNodeInfo = InNodeInfo;
	
	// NodeInfo의 Config가 비어있으면 DefaultRoomConfig 사용
	if (CurrentNodeInfo.Config.TargetEventCounts.Num() <= 0 && DefaultRoomConfig.TargetEventCounts.Num() > 0)
	{
		CurrentNodeInfo.Config.TargetEventCounts = DefaultRoomConfig.TargetEventCounts;
	}

	if (!IsValid(CurrentNodeInfo.Config.StateTree) && IsValid(DefaultRoomConfig.StateTree))
	{
		CurrentNodeInfo.Config.StateTree = DefaultRoomConfig.StateTree;
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
