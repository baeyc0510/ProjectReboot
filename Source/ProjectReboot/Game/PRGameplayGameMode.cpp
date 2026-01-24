// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameMode.h"
#include "PRGameplayGameState.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRMapManagerSubsystem.h"
#include "ProjectReboot/Room/PRRoomController.h"

void APRGameplayGameMode::OnGameplayEvent(const FGameplayTag& EventTag, int32 Delta)
{
	APRGameplayGameState* GS = GetGameState<APRGameplayGameState>();
	if (!GS)
	{
		return;
	}

	GS->AddEventCount(EventTag, Delta);

	// 클리어 조건 판정: 모든 목표 이벤트 수치 달성 여부
	bool bAllGoalsMet = true;
	if (CurrentRoomConfig.TargetEventCounts.Num() <= 0)
	{
		bAllGoalsMet = false;
	}
	else
	{
		for (const TPair<FGameplayTag, int32>& Goal : CurrentRoomConfig.TargetEventCounts)
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
	if (UPRMapManagerSubsystem* MapManager = GetWorld()->GetSubsystem<UPRMapManagerSubsystem>())
	{
		MapManager->CallOrRegister_RoomReady(TargetRoomIndex, 
			FOnRoomReadySignature::FDelegate::CreateUObject(this, &APRGameplayGameMode::OnNextRoomReady));
	}
	// TODO 해당 레벨 인스턴스로 텔레포트
}

void APRGameplayGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (UPRMapManagerSubsystem* MapManager = GetWorld()->GetSubsystem<UPRMapManagerSubsystem>())
	{
		MapManager->GenerateRoomConfigs();
		// TODO: 실제 레벨 인스턴스 생성
		
		MoveToNextRoom(0);
	}
}

void APRGameplayGameMode::OnRoomCleared()
{
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->SendRoomEvent(TAG_Event_Room_Clear);
	}
	UE_LOG(LogTemp,Warning,TEXT("Gameplay room cleared!"));
}

void APRGameplayGameMode::OnNextRoomReady(APRRoomController* RoomController, const FPRRoomConfig& RoomConfig)
{
	CurrentRoomConfig = RoomConfig;
	
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->CurrentRoomController = RoomController;
	}
	
	if (RoomController)
	{
		RoomController->InitRoom(RoomConfig);
		RoomController->ActivateRoom();
		
		UE_LOG(LogTemp,Warning,TEXT("Room %d ready!"), RoomController->GetRoomIndex());
	}
}
