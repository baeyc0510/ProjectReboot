// Fill out your copyright notice in the Description page of Project Settings.

#include "PREquipmentStandManager.h"
#include "PREquipmentStand.h"
#include "PREquipmentPreviewActor.h"
#include "RogueliteBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentActionSet.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"

APREquipmentStandManager::APREquipmentStandManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APREquipmentStandManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeStands();
}

void APREquipmentStandManager::SelectStand(APREquipmentStand* Stand, APawn* PlayerPawn)
{
	if (!IsValid(Stand) || !IsValid(PlayerPawn))
	{
		return;
	}

	// 이미 선택된 거치대인지 확인
	if (CurrentSelectedStandInfo.EquipmentStand == Stand)
	{
		return;
	}

	// 기존 선택된 거치대가 있으면 해제
	if (IsValid(CurrentSelectedStandInfo.EquipmentStand))
	{
		CurrentSelectedStandInfo.EquipmentStand->Deselect();
		UnequipFromActor(PlayerPawn, CurrentSelectedStandInfo.EquipmentStand->GetEquipmentActionSet());

		// 기존 PreviewActor 다시 표시
		if (IsValid(CurrentSelectedStandInfo.PreviewActor))
		{
			CurrentSelectedStandInfo.PreviewActor->SetActorHiddenInGame(false);
		}
	}

	// 새 거치대의 Info 찾기
	FPREquipmentStandInfo* NewStandInfo = ManagedStandInfoList.FindByPredicate(
		[Stand](const FPREquipmentStandInfo& Info) { return Info.EquipmentStand == Stand; });

	if (!NewStandInfo)
	{
		return;
	}

	// 새 장비 장착
	EquipPartsToActor(PlayerPawn, Stand->GetEquipmentActionSet());

	// 새 거치대 선택
	Stand->Select();
	CurrentSelectedStandInfo = *NewStandInfo;

	// 새 PreviewActor 숨김
	if (IsValid(CurrentSelectedStandInfo.PreviewActor))
	{
		CurrentSelectedStandInfo.PreviewActor->SetActorHiddenInGame(true);
	}
}

void APREquipmentStandManager::DeselectCurrentStand()
{
	if (!IsValid(CurrentSelectedStandInfo.EquipmentStand))
	{
		return;
	}

	CurrentSelectedStandInfo.EquipmentStand->Deselect();

	// PreviewActor 다시 표시
	if (IsValid(CurrentSelectedStandInfo.PreviewActor))
	{
		CurrentSelectedStandInfo.PreviewActor->SetActorHiddenInGame(false);
	}

	CurrentSelectedStandInfo = FPREquipmentStandInfo();
}

void APREquipmentStandManager::InitializeStands()
{
	for (const FPREquipmentStandInfo& StandInfo : ManagedStandInfoList)
	{
		if (IsValid(StandInfo.EquipmentStand))
		{
			StandInfo.EquipmentStand->SetOwningManager(this);
		}
	}
}

void APREquipmentStandManager::EquipPartsToActor(APawn* Target,  const UPREquipmentActionSet* EquipmentActionSet)
{
	if (!IsValid(Target) || !IsValid(EquipmentActionSet))
	{
		return;
	}

	EquipmentActionSet->AcquireActionSet(Target);
}

void APREquipmentStandManager::UnequipFromActor(APawn* Target, const UPREquipmentActionSet* EquipmentActionSet)
{
	if (!IsValid(Target) || !IsValid(EquipmentActionSet) || !EquipmentActionSet->GetPrimarySlotTag().IsValid())
	{
		return;
	}

	// Roguelite 시스템을 통해 장비 제거 (모든 Attached Parts 포함)
	UPREquipmentBlueprintLibrary::RemoveEquipmentInstance(Target, EquipmentActionSet->GetPrimarySlotTag());
}
