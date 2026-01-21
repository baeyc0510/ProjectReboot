// Fill out your copyright notice in the Description page of Project Settings.


#include "PRMapManagerSubsystem.h"

#include "PRRoomController.h"
#include "ProjectReboot/Game/PRGameplayGameMode.h"

void UPRMapManagerSubsystem::GenerateRoomConfigs()
{
	RunMap.Empty();
	PendingRoomReadyRequests.Empty();

	// 시작 방 생성 (Index 0)
	FRoomInstanceInfo StartNode;
	StartNode.RoomIndex =0;
	StartNode.Depth = 0;
	
	if (APRGameplayGameMode* GM = GetWorld()->GetAuthGameMode<APRGameplayGameMode>())
	{
		StartNode.Config = GM->DefaultRoomConfig;
	}
	
	RunMap.Add(StartNode.RoomIndex, StartNode);
	
	// TODO: 절차적 생성

	UE_LOG(LogTemp, Warning, TEXT("PR Subsystem: Full Run Map Generated with %d Rooms"), RunMap.Num());
}

void UPRMapManagerSubsystem::CallOrRegister_RoomReady(int32 RoomIndex, FOnRoomReadySignature::FDelegate&& OnReadyCallback)
{
	check(RunMap.Find(RoomIndex));
	
	if (!RunMap.Find(RoomIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid room index"));
		return;
	}

	if (RunMap[RoomIndex].Controller)
	{
		OnReadyCallback.ExecuteIfBound(RunMap[RoomIndex].Controller, RunMap[RoomIndex].Config);
	}
	else
	{
		FOnRoomReadySignature& Delegate = PendingRoomReadyRequests.FindOrAdd(RoomIndex);
		Delegate.Add(MoveTemp(OnReadyCallback));
	}
}

void UPRMapManagerSubsystem::RegisterRoomController(int32 RoomIndex, APRRoomController* InController)
{
	check(RunMap.Find(RoomIndex));
	
	if (!RunMap.Find(RoomIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid room index"));
		return;
	}
	if (!IsValid(InController))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid room RoomController"));
		return;
	}
	
	FRoomInstanceInfo& Info = RunMap[RoomIndex];
	Info.Controller = InController;
	
	if (FOnRoomReadySignature* Delegate = PendingRoomReadyRequests.Find(RoomIndex))
	{
		if (Delegate->IsBound())
		{
			Delegate->Broadcast(InController,Info.Config);
			Delegate->Clear();
		}
		
		PendingRoomReadyRequests.Remove(RoomIndex);
	}
}
