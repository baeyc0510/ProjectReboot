// Fill out your copyright notice in the Description page of Project Settings.

#include "PRRewardActor.h"

#include "RoguelitePoolPreset.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "ProjectReboot/UI/ActionList/PRActionDecisionPanel.h"
#include "ProjectReboot/UI/PRUIManagerSubsystem.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "Blueprint/UserWidget.h"

APRRewardActor::APRRewardActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APRRewardActor::SetRewardPoolPreset(URoguelitePoolPreset* InPoolPreset)
{
	if (InPoolPreset)
	{
		RewardPoolPreset = InPoolPreset;	
	}
}

void APRRewardActor::BeginPlay()
{
	Super::BeginPlay();
}

bool APRRewardActor::CanInteract(APawn* Interactor) const
{
	return bIsInteractable && IsValid(RewardPoolPreset);
}

void APRRewardActor::Interact(APawn* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}

	ShowRewardPanel(Interactor);
}

FText APRRewardActor::GetInteractionText() const
{
	return InteractionText;
}

void APRRewardActor::ShowRewardPanel(APawn* Interactor)
{
	if (!IsValid(DecisionPanelClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: DecisionPanelClass is not set"));
		return;
	}

	// RogueliteSubsystem에서 액션 쿼리
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: RogueliteSubsystem is invalid"));
		return;
	}

	// 보상 액션 목록 쿼리
	TArray<URogueliteActionData*> RewardActions = RogueliteSubsystem->QuerySimple(RewardPoolPreset, RewardCount);
	if (RewardActions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: No reward actions found"));
		return;
	}

	// PlayerController 가져오기
	APlayerController* PC = Cast<APlayerController>(Interactor->GetController());
	if (!IsValid(PC))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: PlayerController is invalid"));
		return;
	}

	// UIManager를 통해 패널 생성 및 표시
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: LocalPlayer is invalid"));
		return;
	}

	UPRUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPRUIManagerSubsystem>();
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: UIManager is invalid"));
		return;
	}

	CurrentPanel = Cast<UPRActionDecisionPanel>(UIManager->PushUI(DecisionPanelClass));
	if (!IsValid(CurrentPanel))
	{
		UE_LOG(LogTemp, Warning, TEXT("PRRewardActor::ShowRewardPanel: Failed to create decision panel"));
		return;
	}

	// 패널 설정
	CurrentPanel->SetSourceActor(Interactor);
	CurrentPanel->SetDecisionList(RewardActions);

	// 선택 확정 델리게이트 바인딩
	CurrentPanel->OnActionConfirmed.AddDynamic(this, &APRRewardActor::HandleRewardConfirmed);

	// 상호작용 비활성화 (중복 선택 방지)
	bIsInteractable = false;
}

void APRRewardActor::HandleRewardConfirmed(URogueliteActionData* SelectedAction)
{
	// 보상 선택 이벤트 전송
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
		{
			GameState->SendRoomEvent(TAG_Event_Reward_Selected);
		}
	}

	// UIManager를 통해 패널 제거
	if (IsValid(CurrentPanel))
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (IsValid(PC))
		{
			ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
			if (IsValid(LocalPlayer))
			{
				UPRUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UPRUIManagerSubsystem>();
				if (IsValid(UIManager))
				{
					UIManager->PopUI(CurrentPanel);
				}
			}
		}
	}

	// 패널 참조 해제
	CurrentPanel = nullptr;

	// 액터 제거
	Destroy();
}
