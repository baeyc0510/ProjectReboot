// Fill out your copyright notice in the Description page of Project Settings.

#include "PREquipmentStand.h"
#include "PREquipmentStandManager.h"
#include "ProjectReboot/Equipment/PREquipmentActionSet.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Equipment/EquipmentInstance.h"
#include "ProjectReboot/Interaction/PRBillboardWidgetComponent.h"

APREquipmentStand::APREquipmentStand()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	PreviewAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewAttachComponent"));
	PreviewAttachComponent->SetupAttachment(RootSceneComponent);

	InteractionPromptWidget = CreateDefaultSubobject<UPRBillboardWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(RootSceneComponent);
	InteractionPromptWidget->SetVisibility(false);
}

void APREquipmentStand::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializePreviewVisuals();
}

void APREquipmentStand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(PreviewInstance))
	{
		PreviewInstance->Uninitialize();
		PreviewInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

bool APREquipmentStand::CanInteract(APawn* Interactor) const
{
	return bCanInteract && !bIsSelected;
}

void APREquipmentStand::Interact(APawn* Interactor)
{
	if (!IsValid(OwningManager.Get()))
	{
		return;
	}

	OwningManager->SelectStand(this, Interactor);
}

FText APREquipmentStand::GetInteractionText() const
{
	return NSLOCTEXT("Interaction", "SelectEquipment", "장비 선택");
}

void APREquipmentStand::OnGainInteractFocus(APawn* Interactor)
{
	if (IsValid(InteractionPromptWidget))
	{
		InteractionPromptWidget->SetVisibility(true);
	}
}

void APREquipmentStand::OnLoseInteractFocus(APawn* Interactor)
{
	if (IsValid(InteractionPromptWidget))
	{
		InteractionPromptWidget->SetVisibility(false);
	}
}

void APREquipmentStand::Select()
{
	bIsSelected = true;
	bCanInteract = false;
	SetActorHiddenInGame(true);
}

void APREquipmentStand::Deselect()
{
	bIsSelected = false;
	bCanInteract = true;
	SetActorHiddenInGame(false);
}

void APREquipmentStand::SetOwningManager(APREquipmentStandManager* Manager)
{
	OwningManager = Manager;
}

void APREquipmentStand::InitializePreviewVisuals()
{
	if (!IsValid(EquipmentActionSet) || !IsValid(EquipmentActionSet->PrimaryAction))
	{
		return;
	}

	UPREquipActionData* PrimaryAction = EquipmentActionSet->PrimaryAction;

	// EquipmentInstanceType이 설정되어 있으면 해당 클래스 사용, 없으면 기본 클래스
	TSubclassOf<UEquipmentInstance> InstanceClass = PrimaryAction->EquipmentInstanceType;
	if (!InstanceClass)
	{
		InstanceClass = UEquipmentInstance::StaticClass();
	}

	// PreviewInstance 생성 및 초기화
	PreviewInstance = NewObject<UEquipmentInstance>(this, InstanceClass);
	PreviewInstance->Initialize(PreviewAttachComponent, PrimaryAction);

	// 파트 부착
	for (UPREquipActionData* PartAction : EquipmentActionSet->PartActions)
	{
		if (IsValid(PartAction))
		{
			PreviewInstance->AttachPart(PartAction);
		}
	}

	// 비주얼 새로고침
	PreviewInstance->RefreshVisuals();
}
