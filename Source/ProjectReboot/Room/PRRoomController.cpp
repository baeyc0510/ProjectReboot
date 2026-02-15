// Fill out your copyright notice in the Description page of Project Settings.


#include "PRRoomController.h"

#include "PRRoomWorldSubsystem.h"
#include "PRRoomDoor.h"
#include "PRRewardActor.h"
#include "PRStageManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "Misc/PackageName.h"
#include "StateTree/PRRoomStateTreeComponent.h"


// Sets default values
APRRoomController::APRRoomController()
{
	StateTreeComponent = CreateDefaultSubobject<UPRRoomStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
}

void APRRoomController::InitRoom(const FPRRoomNodeInfo& InNodeInfo)
{
	NodeInfo = InNodeInfo;
	if (NodeInfo.RoomIndex >= 0)
	{
		RoomIndex = NodeInfo.RoomIndex;
	}

	// 총 웨이브 수 설정
	TotalWaveCount = NodeInfo.SpawnInfo.Waves.Num();

	if (StateTreeComponent)
	{
		StateTreeComponent->SetStateTreeAsset(NodeInfo.FlowConfig.StateTree);
	}

	// 문에 다음 방 정보 할당
	SpawnDoors(NodeInfo.NextRoomIndices);

	// 문 비활성화 (클리어 후 ShowDoors에서 활성화)
	SetDoorsInteractable(false);
}

void APRRoomController::ActivateRoom()
{
	// 웨이브가 있으면 WaveClear 이벤트 델리게이트 바인딩
	if (NodeInfo.SpawnInfo.Waves.Num() > 0)
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
			{
				WaveClearHandle = GameState->OnEventCountChanged.AddUObject(
					this, &APRRoomController::HandleWaveClearEvent);

				UE_LOG(LogTemp, Log, TEXT("PRRoomController::ActivateRoom: Bound to WaveClear event (Room %d, %d waves)"),
					RoomIndex, NodeInfo.SpawnInfo.Waves.Num());
			}
		}
	}

	// StateTree 시작
	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
		UE_LOG(LogTemp, Log, TEXT("PRRoomController::ActivateRoom: StateTree started (Room %d)"), RoomIndex);
	}
}

int32 APRRoomController::GetRoomIndex() const
{
	if (RoomIndex >= 0)
	{
		return RoomIndex;
	}

	return ExtractRoomIndexFromLevelName();
}

void APRRoomController::SetDoorsInteractable(bool bEnabled)
{
	for (APRRoomDoor* Door : Doors)
	{
		if (IsValid(Door))
		{
			Door->SetInteractable(bEnabled);
		}
	}
}

void APRRoomController::ShowDoors()
{
	for (APRRoomDoor* Door : Doors)
	{
		if (!IsValid(Door))
		{
			continue;
		}

		Door->SetVisibility(true);
	}

	// 사운드 재생
	FVector SoundLocation = IsValid(DoorSpawnPoint) ? DoorSpawnPoint->GetActorLocation() : GetActorLocation();
	if (IsValid(DoorActivationSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, DoorActivationSound, SoundLocation);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomController: Doors shown (Room %d, Count %d)"), RoomIndex, Doors.Num());
}

void APRRoomController::HideAndDisableDoors()
{
	for (APRRoomDoor* Door : Doors)
	{
		if (!IsValid(Door))
		{
			continue;
		}

		Door->SetVisibility(false);
		Door->SetInteractable(false);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomController: Doors hidden (Room %d)"), RoomIndex);
}

void APRRoomController::SpawnDoors(const TArray<int32>& NextRoomIndices)
{
	// 기존 문 제거
	DestroyDoors();

	if (!DoorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomController: DoorClass is not set (Room %d)"), RoomIndex);
		return;
	}

	const int32 DoorCount = NextRoomIndices.Num();
	if (DoorCount == 0)
	{
		return;
	}

	// 스폰 중앙 위치/회전
	FVector CenterLocation = IsValid(DoorSpawnPoint) ? DoorSpawnPoint->GetActorLocation() : GetActorLocation();
	FRotator CenterRotation = IsValid(DoorSpawnPoint) ? DoorSpawnPoint->GetActorRotation() : GetActorRotation();
	FVector RightVector = CenterRotation.RotateVector(FVector::RightVector);

	// 중앙 기준 좌우 대칭 배치 (오프셋 계산)
	float TotalWidth = (DoorCount - 1) * DoorSpacing;
	float StartOffset = -TotalWidth * 0.5f;

	UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < DoorCount; i++)
	{
		float Offset = StartOffset + i * DoorSpacing;
		FVector SpawnLocation = CenterLocation + RightVector * Offset;

		APRRoomDoor* Door = GetWorld()->SpawnActor<APRRoomDoor>(DoorClass, SpawnLocation, CenterRotation, SpawnParams);
		if (!IsValid(Door))
		{
			continue;
		}

		// 다음 방 정보 할당
		const int32 TargetRoomIndex = NextRoomIndices[i];
		Door->SetTargetRoomIndex(TargetRoomIndex);

		if (IsValid(StageManager))
		{
			if (const FPRRoomNodeInfo* TargetNodeInfo = StageManager->GetRoomNodeInfo(TargetRoomIndex))
			{
				Door->SetDisplayInfo(TargetNodeInfo->RoomType, TargetNodeInfo->RewardCategory);
			}
		}

		// 초기 숨김 상태 (ShowDoors에서 표시)
		Door->SetActorHiddenInGame(true);
		Door->SetActorEnableCollision(false);
		Door->SetInteractable(false);

		Doors.Add(Door);
		RegisterSpawnedActor(Door);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomController: Spawned %d doors (Room %d)"), DoorCount, RoomIndex);
}

void APRRoomController::DestroyDoors()
{
	for (APRRoomDoor* Door : Doors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}
	Doors.Empty();
}

FTransform APRRoomController::GetPlayerSpawnTransform() const
{
	if (IsValid(PlayerSpawnPoint))
	{
		return PlayerSpawnPoint->GetActorTransform();
	}

	// 스폰 포인트가 없으면 RoomController 위치 사용
	return GetActorTransform();
}

AActor* APRRoomController::SpawnReward(TSubclassOf<AActor> RewardActorClass)
{
	if (!RewardActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomController::SpawnReward: RewardActorClass is null"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	// 보상 스폰 위치 결정
	FTransform SpawnTransform = IsValid(RewardSpawnPoint) ? RewardSpawnPoint->GetActorTransform() : GetActorTransform();

	// 보상 액터 스폰
	AActor* SpawnedActor = World->SpawnActor<AActor>(RewardActorClass, SpawnTransform);
	if (!IsValid(SpawnedActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomController::SpawnReward: Failed to spawn reward actor"));
		return nullptr;
	}

	// RewardPoolPreset 설정 (NodeInfo에서 가져옴)
	if (APRRewardActor* RewardActor = Cast<APRRewardActor>(SpawnedActor))
	{
		RewardActor->SetRewardPoolPreset(NodeInfo.RewardPoolPreset);
	}

	// 스폰 액터 등록
	RegisterSpawnedActor(SpawnedActor);

	return SpawnedActor;
}

bool APRRoomController::IncrementWaveIndex()
{
	if (CurrentWaveIndex + 1 < TotalWaveCount)
	{
		CurrentWaveIndex++;
		UE_LOG(LogTemp, Log, TEXT("PRRoomController::IncrementWaveIndex: Wave %d/%d"), CurrentWaveIndex + 1, TotalWaveCount);
		return true;
	}

	// 마지막 웨이브
	UE_LOG(LogTemp, Log, TEXT("PRRoomController::IncrementWaveIndex: Last wave completed"));
	return false;
}

void APRRoomController::HandleWaveClearEvent(const FGameplayTag& EventTag, int32 Count)
{
	// Wave Clear 이벤트만 처리
	if (EventTag != TAG_Event_Wave_Cleared)
	{
		return;
	}

	// 웨이브 인덱스 증가
	IncrementWaveIndex();

	UE_LOG(LogTemp, Log, TEXT("PRRoomController::HandleWaveClearEvent: Wave cleared, incremented to %d/%d"),
		CurrentWaveIndex + 1, GetTotalWaveCount());
}

void APRRoomController::BeginPlay()
{
	Super::BeginPlay();

	if (RoomIndex < 0)
	{
		RoomIndex = ExtractRoomIndexFromLevelName();
	}
	
	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		RoomSubsystem->RegisterRoomController(GetRoomIndex(), this);
	}
	
	// // 1 프레임 지연 (다른 액터들 먼저 초기화)
	// GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	// {
	// 	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	// 	{
	// 		RoomSubsystem->RegisterRoomController(GetRoomIndex(), this);
	// 	}
	// }));
}

int32 APRRoomController::ExtractRoomIndexFromLevelName() const
{
	const ULevel* Level = GetLevel();
	if (!IsValid(Level))
	{
		return 0;
	}

	const UPackage* Package = Level->GetOutermost();
	if (!IsValid(Package))
	{
		return 0;
	}

	const FString PackageName = Package->GetName();
	const FString ShortName = FPackageName::GetShortName(PackageName);
	const FString Token = TEXT("Room_");

	// "Room_" 위치 찾기
	const int32 TokenIndex = ShortName.Find(Token, ESearchCase::IgnoreCase);
	checkf(TokenIndex != INDEX_NONE,
		TEXT("PRRoomController: Level name must contain '%s' (Current: %s)"),
		*Token, *ShortName);

	const int32 NumberStart = TokenIndex + Token.Len();
	if (NumberStart >= ShortName.Len())
	{
		return 0;
	}

	FString NumberString;
	for (int32 i = NumberStart; i < ShortName.Len(); i++)
	{
		const TCHAR Char = ShortName[i];
		if (!FChar::IsDigit(Char))
		{
			break;
		}

		NumberString.AppendChar(Char);
	}

	return NumberString.IsEmpty() ? 0 : FCString::Atoi(*NumberString);
}

void APRRoomController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 동적 스폰 액터 정리
	DestroyAllSpawnedActors();
	// 웨이브 클리어 델리게이트 해제
	if (WaveClearHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
			{
				GameState->OnEventCountChanged.Remove(WaveClearHandle);
			}
		}
		WaveClearHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void APRRoomController::RegisterSpawnedActor(AActor* Actor)
{
	if (IsValid(Actor))
	{
		SpawnedActors.Add(Actor);
	}
}

void APRRoomController::DestroyAllSpawnedActors()
{
	for (TObjectPtr<AActor>& Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}
