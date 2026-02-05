// Fill out your copyright notice in the Description page of Project Settings.


#include "PRRoomController.h"

#include "PRRoomWorldSubsystem.h"
#include "PRRoomStateTreeComponent.h"
#include "PRRoomDoor.h"
#include "PRStageManagerSubsystem.h"
#include "Components/StateTreeComponent.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "Misc/PackageName.h"


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

	if (StateTreeComponent)
	{
		StateTreeComponent->SetStateTreeAsset(NodeInfo.Config.StateTree);
	}

	SetEntryDoorsInteractable(false);
	SetExitDoorsInteractable(false);
}

void APRRoomController::ActivateRoom()
{
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

void APRRoomController::SetEntryDoorsInteractable(bool bEnabled)
{
	for (AActor* DoorActor : EntryDoors)
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

void APRRoomController::SetExitDoorsInteractable(bool bEnabled)
{
	for (AActor* DoorActor : ExitDoors)
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

void APRRoomController::AssignExitDoors(const TArray<int32>& NextRoomIndices)
{
	UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this);

	for (int32 i = 0; i < ExitDoors.Num(); i++)
	{
		AActor* DoorActor = ExitDoors[i];
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

void APRRoomController::BeginPlay()
{
	Super::BeginPlay();

	if (RoomIndex < 0)
	{
		RoomIndex = ExtractRoomIndexFromLevelName();
	}

	// 1 프레임 지연 (다른 액터들 먼저 초기화)
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
		{
			RoomSubsystem->RegisterRoomController(GetRoomIndex(), this);
		}
	}));
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
	checkf(ShortName.StartsWith(Token, ESearchCase::IgnoreCase),
		TEXT("PRRoomController: Level name must start with '%s' (Current: %s)"),
		*Token, *ShortName);

	const int32 NumberStart = Token.Len();
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