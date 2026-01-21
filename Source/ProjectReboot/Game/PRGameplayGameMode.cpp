// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameMode.h"
#include "PRGameplayGameState.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRMapManagerSubsystem.h"
#include "ProjectReboot/Room/PRRoomController.h"

void APRGameplayGameMode::OnEnemyKilled()
{
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->CurrentKillCount++;

		// 클리어 조건 판정: 현재 킬 수가 목표치에 도달했는가?
		if (GS->CurrentKillCount >= CurrentRoomConfig.TargetKillCount)
		{
			OnRoomCleared();
		}	
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
