// PREnemyStatusWidget.cpp
#include "PREnemyStatusWidget.h"

#include "PREnemyStatusViewModel.h"
#include "Components/ProgressBar.h"

void UPREnemyStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPREnemyStatusWidget::NativeDestruct()
{
	UnbindViewModel();
	Super::NativeDestruct();
}

void UPREnemyStatusWidget::BindViewModel(UPREnemyStatusViewModel* TargetViewModel)
{
	UnbindViewModel();

	ViewModel = TargetViewModel;
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnHealthChanged.AddDynamic(this, &UPREnemyStatusWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.AddDynamic(this, &UPREnemyStatusWidget::HandleShieldChanged);

	ApplyInitialState();
}

void UPREnemyStatusWidget::UnbindViewModel()
{
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnHealthChanged.RemoveDynamic(this, &UPREnemyStatusWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.RemoveDynamic(this, &UPREnemyStatusWidget::HandleShieldChanged);

	ViewModel = nullptr;
}

void UPREnemyStatusWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
}

void UPREnemyStatusWidget::HandleHealthChanged(float Current, float Max)
{
	if (HealthBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		HealthBar->SetPercent(Percent);
	}
}

void UPREnemyStatusWidget::HandleShieldChanged(float Current, float Max)
{
	if (ShieldBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		ShieldBar->SetPercent(Percent);
		ShieldBar->SetVisibility(Max > KINDA_SMALL_NUMBER ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
