// Fill out your copyright notice in the Description page of Project Settings.

#include "PRRoomDoor.h"

#include "PRStageManagerSubsystem.h"

APRRoomDoor::APRRoomDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionText = FText::FromString(TEXT("다음 방"));
}

bool APRRoomDoor::CanInteract(APawn* Interactor) const
{
	return bIsInteractable && TargetRoomIndex >= 0;
}

void APRRoomDoor::Interact(APawn* Interactor)
{
	if (!bIsInteractable)
	{
		return;
	}

	if (TargetRoomIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRoomDoor: No target room index assigned for door %s"), *GetName());
		return;
	}

	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->EnterRoomByIndex(TargetRoomIndex);
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
