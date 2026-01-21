// Fill out your copyright notice in the Description page of Project Settings.


#include "PRRoomController.h"

#include "PRMapManagerSubsystem.h"
#include "PRRoomStateTreeComponent.h"
#include "Components/StateTreeComponent.h"


// Sets default values
APRRoomController::APRRoomController()
{
	StateTreeComponent = CreateDefaultSubobject<UPRRoomStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
}

void APRRoomController::InitRoom(const FPRRoomConfig& InRoomConfig)
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SetStateTreeAsset(InRoomConfig.StateTree);	
	}
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
	// TODO: Level 이름 기반 Index 추출 (절차적 맵 생성에 의한 동적 Index)
	return 0;
}

void APRRoomController::BeginPlay()
{
	Super::BeginPlay();
	
	// 1 프레임 지연 (다른 액터들 먼저 초기화)
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		if (UPRMapManagerSubsystem* MapManager = GetWorld()->GetSubsystem<UPRMapManagerSubsystem>())
		{
			MapManager->RegisterRoomController(GetRoomIndex(),this);
		}	
	}));
}