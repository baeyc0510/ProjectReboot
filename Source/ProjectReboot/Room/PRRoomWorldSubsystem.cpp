// Fill out your copyright notice in the Description page of Project Settings.

#include "PRRoomWorldSubsystem.h"
#include "PRRoomController.h"
#include "PRStageManagerSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "ProjectReboot/Character/PRPlayerCharacter.h"

/*~ USubsystem Interface ~*/

void UPRRoomWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPRRoomWorldSubsystem::Deinitialize()
{
	// 월드 종료 시 자동 정리
	UnloadAllRooms();
	RoomMap.Empty();
	PendingRoomReadyRequests.Empty();
	CurrentRoomController = nullptr;
	CurrentRoomIndex = -1;

	Super::Deinitialize();
}

/*~ Level Instance ~*/

void UPRRoomWorldSubsystem::EnterRoom(int32 RoomIndex, const FPRRoomNodeInfo& NodeInfo)
{
	// 이전 방 언로드 (현재 방이 유효하고, 새 방과 다른 경우)
	if (CurrentRoomIndex != INDEX_NONE && CurrentRoomIndex != RoomIndex)
	{
		UnloadRoom(CurrentRoomIndex);
		UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Unloaded previous room %d"), CurrentRoomIndex);
	}

	// 방 위치 계산 (추후 맵 레이아웃에 따라 수정 가능)
	const FVector RoomLocation = FVector(RoomIndex * 5000.0f, 0.0f, 0.0f);

	// 레벨 인스턴스 로드
	LoadRoomTemplate(RoomIndex, NodeInfo.Template, RoomLocation);

	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Entering room %d"), RoomIndex);
}

void UPRRoomWorldSubsystem::LoadRoomTemplate(int32 RoomIndex, TSoftObjectPtr<UWorld> Template, const FVector& Location)
{
	if (Template.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomWorldSubsystem: Cannot load room %d - template is null"), RoomIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("PRRoomWorldSubsystem: Cannot load room %d - no world"), RoomIndex);
		return;
	}

	// 기존 레벨 인스턴스가 있으면 언로드
	if (LoadedLevelInstances.Contains(RoomIndex))
	{
		UnloadRoom(RoomIndex);
	}

	// 레벨 인스턴스명 생성
	const FString LevelInstanceName = FString::Printf(TEXT("Room_%d"), RoomIndex);

	// 레벨 인스턴스 로드
	bool bSuccess = false;
	ULevelStreamingDynamic* LevelInstance = ULevelStreamingDynamic::LoadLevelInstance(
		World,
		Template.ToString(),
		Location,
		FRotator::ZeroRotator,
		bSuccess,
		LevelInstanceName
	);

	if (!bSuccess || !IsValid(LevelInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("PRRoomWorldSubsystem: Failed to load room %d from %s"), RoomIndex, *Template.ToString());
		return;
	}

	// 로드 완료 콜백 등록
	PendingLoadRoomIndex = RoomIndex;
	LevelInstance->OnLevelLoaded.AddDynamic(this, &UPRRoomWorldSubsystem::OnLevelInstanceLoaded);

	// 레벨 인스턴스 저장
	LoadedLevelInstances.Add(RoomIndex, LevelInstance);

	// 방 정보 초기화
	if (!RoomMap.Contains(RoomIndex))
	{
		FRoomInstanceInfo NewInfo;
		NewInfo.RoomIndex = RoomIndex;
		RoomMap.Add(RoomIndex, NewInfo);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Loading room %d from %s"), RoomIndex, *Template.ToString());
}

void UPRRoomWorldSubsystem::UnloadRoom(int32 RoomIndex)
{
	if (TObjectPtr<ULevelStreamingDynamic>* LevelInstance = LoadedLevelInstances.Find(RoomIndex))
	{
		if (IsValid(*LevelInstance))
		{
			(*LevelInstance)->SetIsRequestingUnloadAndRemoval(true);
		}
		LoadedLevelInstances.Remove(RoomIndex);
	}

	// 방 정보에서 컨트롤러 제거
	if (FRoomInstanceInfo* Info = RoomMap.Find(RoomIndex))
	{
		Info->Controller = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Unloaded room %d"), RoomIndex);
}

void UPRRoomWorldSubsystem::UnloadAllRooms()
{
	TArray<int32> RoomIndices;
	LoadedLevelInstances.GetKeys(RoomIndices);

	for (int32 RoomIndex : RoomIndices)
	{
		UnloadRoom(RoomIndex);
	}
}

void UPRRoomWorldSubsystem::OnLevelInstanceLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Level instance loaded for room %d"), PendingLoadRoomIndex);
	// PRRoomController가 BeginPlay에서 RegisterRoomController를 호출하면
	// PendingRoomReadyRequests의 콜백이 실행됨
}

/*~ Room Controller ~*/

void UPRRoomWorldSubsystem::CallOrRegister_RoomReady(int32 RoomIndex, FOnRoomReadySignature::FDelegate&& OnReadyCallback)
{
	if (!RoomMap.Contains(RoomIndex))
	{
		FRoomInstanceInfo NewInfo;
		NewInfo.RoomIndex = RoomIndex;
		RoomMap.Add(RoomIndex, NewInfo);
	}

	FRoomInstanceInfo& Info = RoomMap[RoomIndex];

	if (IsValid(Info.Controller))
	{
		// StageManager에서 NodeInfo 조회
		FPRRoomNodeInfo NodeInfo;
		if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
		{
			if (const FPRRoomNodeInfo* FoundNode = StageManager->GetRoomNodeInfo(RoomIndex))
			{
				NodeInfo = *FoundNode;
			}
		}
		OnReadyCallback.ExecuteIfBound(Info.Controller, NodeInfo);
	}
	else
	{
		FOnRoomReadySignature& Delegate = PendingRoomReadyRequests.FindOrAdd(RoomIndex);
		Delegate.Add(MoveTemp(OnReadyCallback));
	}
}

void UPRRoomWorldSubsystem::RegisterRoomController(int32 RoomIndex, APRRoomController* InController)
{
	if (!IsValid(InController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomWorldSubsystem: Invalid room controller"));
		return;
	}

	if (!RoomMap.Contains(RoomIndex))
	{
		FRoomInstanceInfo NewInfo;
		NewInfo.RoomIndex = RoomIndex;
		RoomMap.Add(RoomIndex, NewInfo);
	}

	FRoomInstanceInfo& Info = RoomMap[RoomIndex];
	Info.Controller = InController;

	// StageManager에서 NodeInfo 조회
	FPRRoomNodeInfo NodeInfo;
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		if (const FPRRoomNodeInfo* FoundNode = StageManager->GetRoomNodeInfo(RoomIndex))
		{
			NodeInfo = *FoundNode;
			Info.StepIndex = NodeInfo.StepIndex;
		}
	}

	if (FOnRoomReadySignature* Delegate = PendingRoomReadyRequests.Find(RoomIndex))
	{
		if (Delegate->IsBound())
		{
			Delegate->Broadcast(InController, NodeInfo);
			Delegate->Clear();
		}
		PendingRoomReadyRequests.Remove(RoomIndex);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Room %d controller registered"), RoomIndex);
}

APRRoomController* UPRRoomWorldSubsystem::GetRoomController(int32 RoomIndex) const
{
	if (const FRoomInstanceInfo* Info = RoomMap.Find(RoomIndex))
	{
		return Info->Controller;
	}
	return nullptr;
}

void UPRRoomWorldSubsystem::SetCurrentRoom(int32 RoomIndex)
{
	CurrentRoomIndex = RoomIndex;
	CurrentRoomController = GetRoomController(RoomIndex);
}

/*~ Player Teleport ~*/

void UPRRoomWorldSubsystem::TeleportPlayerToRoom(int32 RoomIndex, APlayerController* PlayerController)
{
	APRRoomController* RoomController = GetRoomController(RoomIndex);
	if (!IsValid(RoomController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomWorldSubsystem: Cannot teleport - room %d controller not found"), RoomIndex);
		return;
	}

	// 플레이어 컨트롤러가 지정되지 않으면 첫 번째 로컬 플레이어 사용
	if (!IsValid(PlayerController))
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			PlayerController = World->GetFirstPlayerController();
		}
	}

	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomWorldSubsystem: Cannot teleport - no player controller"));
		return;
	}

	APRPlayerCharacter* PlayerCharacter = Cast<APRPlayerCharacter>(PlayerController->GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomWorldSubsystem: Cannot teleport - no pawn"));
		return;
	}

	const FTransform SpawnTransform = RoomController->GetPlayerSpawnTransform();
	PlayerCharacter->TeleportWithoutCameraLag(SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());

	// 현재 방 업데이트
	SetCurrentRoom(RoomIndex);

	UE_LOG(LogTemp, Log, TEXT("PRRoomWorldSubsystem: Teleported player to room %d at %s"),
		RoomIndex, *SpawnTransform.GetLocation().ToString());
}

void UPRRoomWorldSubsystem::TeleportAllPlayersToRoom(int32 RoomIndex)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC))
		{
			TeleportPlayerToRoom(RoomIndex, PC);
		}
	}
}
