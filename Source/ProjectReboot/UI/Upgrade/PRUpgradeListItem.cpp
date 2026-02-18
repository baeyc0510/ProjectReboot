// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeListItem.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "ProjectReboot/Upgrade/PRUpgradeModuleData.h"

void UPRUpgradeListItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (PurchaseButton)
	{
		PurchaseButton->OnPressed.AddDynamic(this, &ThisClass::HandleButtonPressed);
		PurchaseButton->OnReleased.AddDynamic(this, &ThisClass::HandleButtonReleased);
		PurchaseButton->OnHovered.AddDynamic(this, &ThisClass::HandleButtonHovered);
		PurchaseButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleButtonUnhovered);
	}

	// 초기 상태
	UpdateProgressBar(0.0f);
	RefreshDisplay();

	if (LevelUpOverlay)
	{
		LevelUpOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MaxLevelOverlay)
	{
		MaxLevelOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (TB_NextLevel)
	{
		TB_NextLevel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPRUpgradeListItem::NativeDestruct()
{
	// 홀드 중단 및 바인딩 해제
	StopHold(true);

	if (PurchaseButton)
	{
		PurchaseButton->OnPressed.RemoveAll(this);
		PurchaseButton->OnReleased.RemoveAll(this);
		PurchaseButton->OnHovered.RemoveAll(this);
		PurchaseButton->OnUnhovered.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPRUpgradeListItem::InitWidget(UPRUpgradeModuleData* InModule)
{
	SetModule(InModule);
}

void UPRUpgradeListItem::SetModule(UPRUpgradeModuleData* InModule)
{
	Module = InModule;
	RefreshDisplay();
}

void UPRUpgradeListItem::UpdateDisplay()
{
	RefreshDisplay();
}

void UPRUpgradeListItem::UpdateLevelInfo(int32 InCurrentLevel, int32 InMaxLevel, float InNextCost)
{
	CurrentLevel = InCurrentLevel;
	MaxLevel = InMaxLevel;
	NextCost = InNextCost;

	const bool bIsMaxLevel = CurrentLevel >= MaxLevel;

	RefreshLevelText();

	if (TB_NextLevel)
	{
		TB_NextLevel->SetText(FText::Format(NSLOCTEXT("Upgrade", "NextLevel", "{0}"),
			FText::AsNumber(InCurrentLevel + 1)));
	}

	if (CostPanel)
	{
		if (bIsMaxLevel)
		{
			CostPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			CostPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	
	if (TB_NextCost)
	{
		if (bIsMaxLevel)
		{
			TB_NextCost->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			TB_NextCost->SetVisibility(ESlateVisibility::HitTestInvisible);
			TB_NextCost->SetText(FText::Format(NSLOCTEXT("Upgrade", "NextCost", "{0}"),
				FText::AsNumber(InNextCost)));
		}
	}

	// 최대 레벨 도달 시 오버레이 표시 및 버튼 비활성화
	if (MaxLevelOverlay)
	{
		MaxLevelOverlay->SetVisibility(bIsMaxLevel ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (PurchaseButton)
	{
		PurchaseButton->SetIsEnabled(!bIsMaxLevel);
	}
}

void UPRUpgradeListItem::HandleButtonPressed()
{
	// 홀드 시작
	OnItemPressed.Broadcast(Module);
	StartHold();
}

void UPRUpgradeListItem::HandleButtonReleased()
{
	// 홀드 취소
	StopHold(true);
}

void UPRUpgradeListItem::HandleButtonHovered()
{
	// 호버 애니메이션 재생
	if (HoverAnimation)
	{
		PlayAnimationForward(HoverAnimation);
	}

	bIsHovered = true;
	RefreshLevelText();

	const bool bIsMaxLevel = CurrentLevel >= MaxLevel;

	if (LevelUpOverlay)
	{
		LevelUpOverlay->SetVisibility(bIsMaxLevel ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (TB_NextLevel)
	{
		TB_NextLevel->SetVisibility(bIsMaxLevel ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UPRUpgradeListItem::HandleButtonUnhovered()
{
	// 원상 복귀
	if (HoverAnimation)
	{
		PlayAnimationReverse(HoverAnimation);
	}

	bIsHovered = false;
	RefreshLevelText();

	if (LevelUpOverlay)
	{
		LevelUpOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (TB_NextLevel)
	{
		TB_NextLevel->SetVisibility(ESlateVisibility::Collapsed);
	}

	StopHold(true);
}

void UPRUpgradeListItem::RefreshLevelText()
{
	if (!TB_CurrentLevel)
	{
		return;
	}

	if (bIsHovered)
	{
		TB_CurrentLevel->SetText(FText::Format(NSLOCTEXT("Upgrade", "CurrentLevelHover", "Lv. {0}"),
			FText::AsNumber(CurrentLevel)));
	}
	else
	{
		TB_CurrentLevel->SetText(FText::Format(NSLOCTEXT("Upgrade", "CurrentLevel", "Lv. {0} / {1}"),
			FText::AsNumber(CurrentLevel), FText::AsNumber(MaxLevel)));		
	}
}

void UPRUpgradeListItem::StartHold()
{
	// 중복 홀드 방지
	if (bIsHolding)
	{
		return;
	}

	if (!IsValid(Module))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bIsHolding = true;
	HoldElapsed = 0.0f;
	UpdateProgressBar(0.0f);

	// 홀드 진행 타이머 시작
	World->GetTimerManager().SetTimer(HoldTimerHandle, this, &ThisClass::HandleHoldTick, HoldTickInterval, true);
}

void UPRUpgradeListItem::StopHold(bool bResetProgress)
{
	if (!bIsHolding)
	{
		if (bResetProgress)
		{
			UpdateProgressBar(0.0f);
		}
		return;
	}

	bIsHolding = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldTimerHandle);
	}

	if (bResetProgress)
	{
		UpdateProgressBar(0.0f);
	}
}

void UPRUpgradeListItem::HandleHoldTick()
{
	if (!bIsHolding)
	{
		return;
	}

	// 진행도 누적
	HoldElapsed += HoldTickInterval;
	const float Progress = HoldDuration > 0.0f ? HoldElapsed / HoldDuration : 1.0f;
	UpdateProgressBar(FMath::Clamp(Progress, 0.0f, 1.0f));

	if (HoldElapsed >= HoldDuration)
	{
		HandleHoldCompleted();
	}
}

void UPRUpgradeListItem::HandleHoldCompleted()
{
	// 홀드 종료
	StopHold(false);

	if (!IsValid(Module))
	{
		UpdateProgressBar(0.0f);
		return;
	}

	// 홀드 완료 이벤트
	OnItemHoldCompleted.Broadcast(Module);

	UpdateProgressBar(0.0f);
}

void UPRUpgradeListItem::UpdateProgressBar(float Progress)
{
	if (PB_HoldProgress)
	{
		PB_HoldProgress->SetPercent(Progress);
	}
}

void UPRUpgradeListItem::RefreshDisplay()
{
	if (!IsValid(Module))
	{
		if (TB_Name)
		{
			TB_Name->SetText(FText::GetEmpty());
		}

		if (TB_Description)
		{
			TB_Description->SetText(FText::GetEmpty());
		}

		if (Img_Icon)
		{
			Img_Icon->SetBrushFromTexture(nullptr);
		}

		if (TB_CurrentLevel)
		{
			TB_CurrentLevel->SetText(FText::GetEmpty());
		}

		if (TB_NextLevel)
		{
			TB_NextLevel->SetText(FText::GetEmpty());
		}

		if (TB_NextCost)
		{
			TB_NextCost->SetText(FText::GetEmpty());
		}

		return;
	}

	if (TB_Name)
	{
		TB_Name->SetText(Module->GetDisplayName());
	}

	if (TB_Description)
	{
		TB_Description->SetText(Module->GetDescription());
	}

	if (Img_Icon)
	{
		Img_Icon->SetBrushFromTexture(Module->GetIcon());
	}

	RefreshLevelText();
}
