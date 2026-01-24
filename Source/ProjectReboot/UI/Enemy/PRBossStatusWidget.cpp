// PRBossStatusWidget.cpp
#include "PRBossStatusWidget.h"

#include "PREnemyStatusViewModel.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPRBossStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPRBossStatusWidget::NativeDestruct()
{
	UnbindViewModel();
	Super::NativeDestruct();
}

void UPRBossStatusWidget::BindViewModel(UPREnemyStatusViewModel* TargetViewModel)
{
	UnbindViewModel();

	ViewModel = TargetViewModel;
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnEnemyDisplayNameChanged.AddDynamic(this, &UPRBossStatusWidget::HandleEnemyDisplayNameChanged);
	ViewModel->OnHealthChanged.AddDynamic(this, &UPRBossStatusWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.AddDynamic(this, &UPRBossStatusWidget::HandleShieldChanged);

	ApplyInitialState();
}

void UPRBossStatusWidget::UnbindViewModel()
{
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnEnemyDisplayNameChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleEnemyDisplayNameChanged);
	ViewModel->OnHealthChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleShieldChanged);

	ViewModel = nullptr;
}

void UPRBossStatusWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	HandleEnemyDisplayNameChanged(ViewModel->GetEnemyDisplayName());
	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
}

void UPRBossStatusWidget::HandleEnemyDisplayNameChanged(const FText& DisplayName)
{
	if (BossNameText)
	{
		BossNameText->SetText(DisplayName);
	}
}

void UPRBossStatusWidget::HandleHealthChanged(float Current, float Max)
{
	if (HealthBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		HealthBar->SetPercent(Percent);
	}
}

void UPRBossStatusWidget::HandleShieldChanged(float Current, float Max)
{
	if (ShieldBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		ShieldBar->SetPercent(Percent);
		ShieldBar->SetVisibility(Max > KINDA_SMALL_NUMBER ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
