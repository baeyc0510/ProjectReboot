// Fill out your copyright notice in the Description page of Project Settings.


#include "PRRoomController.h"

#include "PRRoomWorldSubsystem.h"
#include "PRRoomDoor.h"
#include "PRRewardActor.h"
#include "PRStageManagerSubsystem.h"
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
	AssignDoorInfo(NodeInfo.NextRoomIndices);

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

	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
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
	for (AActor* DoorActor : Doors)
	{
		if (!IsValid(DoorActor))
		{
			continue;
		}

		if (IPRInteractableInterface* Interactable = Cast<IPRInteractableInterface>(DoorActor))
		{
			Interactable->SetInteractable(bEnabled);
		}
	}
}

void APRRoomController::ShowDoors()
{
	for (AActor* DoorActor : Doors)
	{
		if (!IsValid(DoorActor))
		{
			continue;
		}

		// 문 표시
		DoorActor->SetActorHiddenInGame(false);

		// 충돌 활성화
		DoorActor->SetActorEnableCollision(true);
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomController: Doors shown and enabled (Room %d)"), RoomIndex);
}

void APRRoomController::HideAndDisableDoors()
{
	for (AActor* DoorActor : Doors)
	{
		if (!IsValid(DoorActor))
		{
			continue;
		}

		// 문 숨김
		DoorActor->SetActorHiddenInGame(true);

		// 충돌 비활성화
		DoorActor->SetActorEnableCollision(false);

		// 상호작용 비활성화
		if (IPRInteractableInterface* Interactable = Cast<IPRInteractableInterface>(DoorActor))
		{
			Interactable->SetInteractable(false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PRRoomController: Doors hidden and disabled (Room %d)"), RoomIndex);
}

void APRRoomController::AssignDoorInfo(const TArray<int32>& NextRoomIndices)
{
	UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this);

	for (int32 i = 0; i < Doors.Num(); i++)
	{
		AActor* DoorActor = Doors[i];
		if (!IsValid(DoorActor))
		{
			continue;
		}

		APRRoomDoor* Door = Cast<APRRoomDoor>(DoorActor);
		if (!IsValid(Door))
		{
			continue;
		}

		if (NextRoomIndices.IsValidIndex(i))
		{
			const int32 TargetRoomIndex = NextRoomIndices[i];
			Door->SetTargetRoomIndex(TargetRoomIndex);

			// StageManager에서 NodeInfo 조회하여 표시 정보 설정
			if (IsValid(StageManager))
			{
				if (const FPRRoomNodeInfo* TargetNodeInfo = StageManager->GetRoomNodeInfo(TargetRoomIndex))
				{
					Door->SetDisplayInfo(TargetNodeInfo->RoomType, TargetNodeInfo->RewardCategory);
				}
			}
		}
		else
		{
			Door->ClearAssignment();
		}
	}
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