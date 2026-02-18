// Fill out your copyright notice in the Description page of Project Settings.

#include "PRFieldDropActor.h"

#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "Components/SphereComponent.h"
#include "ProjectReboot/Interaction/PRBillboardWidgetComponent.h"
#include "ProjectReboot/UI/PRBillboardInfoWidget.h"

APRFieldDropActor::APRFieldDropActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootSphereComponent"));
	RootSphereComponent->SetCollisionProfileName(TEXT("Interactable"));
	SetRootComponent(RootSphereComponent);

	InteractionBillboardWidget = CreateDefaultSubobject<UPRBillboardWidgetComponent>(TEXT("InteractionBillboardWidget"));
	InteractionBillboardWidget->SetupAttachment(RootSphereComponent);
}

void APRFieldDropActor::SetActionData(URogueliteActionData* InActionData)
{
	ActionData = InActionData;
	UpdatePromptWidget();
}

bool APRFieldDropActor::CanInteract(APawn* Interactor) const
{
	return bIsInteractable && IsValid(ActionData);
}

void APRFieldDropActor::Interact(APawn* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}

	// RogueliteSubsystem을 통해 액션 즉시 획득
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (IsValid(RogueliteSubsystem))
	{
		RogueliteSubsystem->AcquireAction(ActionData);
	}

	// 상호작용 비활성화 후 제거
	bIsInteractable = false;
	Destroy();
}

FText APRFieldDropActor::GetInteractionText() const
{
	return FText::FromString(TEXT("획득"));
}

void APRFieldDropActor::GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const
{
	OutInfo = FPRInteractionInfo();
	OutInfo.bIsEnabled = CanInteract(Interactor);

	if (IsValid(ActionData))
	{
		OutInfo.DisplayText =  ActionData->GetFormattedTextWithValuesByIndex(ActionData->DisplayName);
		OutInfo.Icon = ActionData->Icon;
	}
}

void APRFieldDropActor::UpdatePromptWidget()
{
	if (!IsValid(InteractionBillboardWidget))
	{
		return;
	}

	// 위젯 초기화 (지연 생성 방지)
	InteractionBillboardWidget->InitWidget();

	UPRBillboardInfoWidget* InfoWidget = Cast<UPRBillboardInfoWidget>(InteractionBillboardWidget->GetWidget());
	if (IsValid(InfoWidget) && IsValid(ActionData))
	{
		FText DisplayName = ActionData->GetFormattedTextWithValuesByIndex(ActionData->DisplayName);
		InfoWidget->SetContent(DisplayName, ActionData->Icon);
	}
}
