// Fill out your copyright notice in the Description page of Project Settings.

#include "PRRoomDoor.h"

#include "PRStageManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/Game/PRGameplayGameMode.h"

APRRoomDoor::APRRoomDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionText = FText::FromString(TEXT("다음 방"));
}

void APRRoomDoor::BeginPlay()
{
	Super::BeginPlay();

	// 초기에는 숨김 상태
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetInteractable(false);

	UE_LOG(LogTemp, Log, TEXT("PRRoomDoor::BeginPlay: Door %s hidden on start"), *GetName());
}

bool APRRoomDoor::CanInteract(APawn* Interactor) const
{
	return bIsInteractable && TargetRoomIndex >= 0;
}

void APRRoomDoor::Interact(APawn* Interactor)
{
	if (!bIsInteractable || !GetWorld())
	{
		return;
	}

	if (TargetRoomIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomDoor: No target room index assigned for door %s"), *GetName());
		return;
	}
	
	if (APRGameplayGameMode* GM = GetWorld()->GetAuthGameMode<APRGameplayGameMode>())
	{
		GM->MoveToNextRoom(TargetRoomIndex);
	}
}

FText APRRoomDoor::GetInteractionText() const
{
	return InteractionText;
}

void APRRoomDoor::GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const
{
	OutInfo = FPRInteractionInfo();
	OutInfo.DisplayText = InteractionText;
	OutInfo.bIsEnabled = bIsInteractable && TargetRoomIndex >= 0;
}

void APRRoomDoor::SetInteractable(bool bEnabled)
{
	bIsInteractable = bEnabled;
}

void APRRoomDoor::SetTargetRoomIndex(int32 InTargetRoomIndex)
{
	TargetRoomIndex = InTargetRoomIndex;
}

void APRRoomDoor::SetDisplayInfo(EPRRoomType InRoomType, FGameplayTag InRewardCategory)
{
	DisplayRoomType = InRoomType;
	DisplayRewardCategory = InRewardCategory;
	OnDisplayInfoSet(InRoomType, InRewardCategory);
}

void APRRoomDoor::ClearAssignment()
{
	TargetRoomIndex = -1;
	DisplayRoomType = EPRRoomType::None;
	DisplayRewardCategory = FGameplayTag();
	OnAssignmentCleared();
}
