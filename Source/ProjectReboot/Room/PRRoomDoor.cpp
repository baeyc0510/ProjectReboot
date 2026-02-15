// Fill out your copyright notice in the Description page of Project Settings.

#include "PRRoomDoor.h"

#include "PRStageManagerSubsystem.h"
#include "Components/BoxComponent.h"
#include "ProjectReboot/Game/PRGameplayGameMode.h"
#include "ProjectReboot/Interaction/PRBillboardWidgetComponent.h"
#include "ProjectReboot/UI/PRBillboardInfoWidget.h"

APRRoomDoor::APRRoomDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionText = FText::FromString(TEXT("들어가기"));

	RootBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootBoxComponent"));
	RootBoxComponent->SetCollisionProfileName(TEXT("Interactable"));
	SetRootComponent(RootBoxComponent);

	BillboardWidget = CreateDefaultSubobject<UPRBillboardWidgetComponent>(TEXT("BillboardWidget"));
	BillboardWidget->SetupAttachment(RootBoxComponent);
}

void APRRoomDoor::SetVisibility(bool bIsVisible)
{
	if (bIsVisible)
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		OnDoorVisibilityChanged.Broadcast(true);
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		OnDoorVisibilityChanged.Broadcast(false);
	}
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
	UpdateBillboardFromCategory(InRewardCategory);
	OnDisplayInfoSet(InRoomType, InRewardCategory);
}

void APRRoomDoor::ClearAssignment()
{
	TargetRoomIndex = -1;
	DisplayRoomType = EPRRoomType::Default;
	DisplayRewardCategory = FGameplayTag();

	// 빌보드 위젯 숨김
	if (IsValid(BillboardWidget))
	{
		BillboardWidget->SetVisibility(false);
	}

	OnAssignmentCleared();
}

void APRRoomDoor::UpdateBillboardFromCategory(FGameplayTag InRewardCategory)
{
	if (!IsValid(BillboardWidget))
	{
		return;
	}

	// 매핑에서 카테고리 정보 검색
	const FPRRewardCategoryDisplayInfo* DisplayInfo = RewardCategoryDisplayMap.Find(InRewardCategory);
	if (!DisplayInfo)
	{
		BillboardWidget->SetVisibility(false);
		return;
	}

	// 위젯 초기화 및 콘텐츠 설정
	BillboardWidget->InitWidget();

	UPRBillboardInfoWidget* InfoWidget = Cast<UPRBillboardInfoWidget>(BillboardWidget->GetWidget());
	if (IsValid(InfoWidget))
	{
		InfoWidget->SetContent(DisplayInfo->DisplayText, DisplayInfo->Icon);
	}

	BillboardWidget->SetVisibility(true);
}
