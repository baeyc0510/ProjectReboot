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
	ViewModel->OnVisibilityChanged.AddDynamic(this, &UPREnemyStatusWidget::HandleVisibilityChanged);

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
	ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPREnemyStatusWidget::HandleVisibilityChanged);

	ViewModel = nullptr;
}

void UPREnemyStatusWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	// 초기 상태 동기화
	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
	HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPREnemyStatusWidget::HandleVisibilityChanged(bool bVisible)
{
	// 위젯 가시성 적용
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
