// Fill out your copyright notice in the Description page of Project Settings.

#include "PREquipmentStand.h"
#include "PREquipmentStandManager.h"
#include "ProjectReboot/Equipment/PREquipmentActionSet.h"
#include "ProjectReboot/Interaction/PRBillboardWidgetComponent.h"
#include "ProjectReboot/UI/Equipment/PREquipmentInfoWidget.h"

APREquipmentStand::APREquipmentStand()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	InteractionPromptWidget = CreateDefaultSubobject<UPRBillboardWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(RootSceneComponent);
}

void APREquipmentStand::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	InitializeInfoWidget();
}

void APREquipmentStand::BeginPlay()
{
	Super::BeginPlay();
	InteractionPromptWidget->SetVisibility(false);
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

void APREquipmentStand::InitializeInfoWidget()
{
	if (!IsValidChecked(InteractionPromptWidget) || !IsValidChecked(InfoWidgetClass))
	{
		return;
	}

	if (UPREquipmentInfoWidget* PrevWidget =  Cast<UPREquipmentInfoWidget>(InteractionPromptWidget->GetWidget()))
	{
		PrevWidget->SetEquipmentInfo(EquipmentActionSet);
		return;
	}

	// 위젯 클래스 설정
	InteractionPromptWidget->SetWidgetClass(InfoWidgetClass);

	// 위젯 강제 생성 (visibility가 false일 때 지연 생성 방지)
	InteractionPromptWidget->InitWidget();

	// 위젯 인스턴스 가져와서 정보 설정
	UPREquipmentInfoWidget* InfoWidget = Cast<UPREquipmentInfoWidget>(InteractionPromptWidget->GetWidget());
	if (IsValid(InfoWidget) && IsValid(EquipmentActionSet))
	{
		InfoWidget->SetEquipmentInfo(EquipmentActionSet);
	}
}
