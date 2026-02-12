// PREnemyStatusWidget.cpp
#include "PREnemyStatusWidget.h"

#include "PREnemyStatusViewModel.h"
#include "ProjectReboot/UI/Interfaces/PRProgressBarInterface.h"

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
	ViewModel->OnHealthSegmentChanged.AddDynamic(this, &UPREnemyStatusWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.AddDynamic(this, &UPREnemyStatusWidget::HandleShieldSegmentChanged);
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
	ViewModel->OnHealthSegmentChanged.RemoveDynamic(this, &UPREnemyStatusWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.RemoveDynamic(this, &UPREnemyStatusWidget::HandleShieldSegmentChanged);
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
	HandleHealthSegmentChanged(ViewModel->GetHealthNumSegments(), ViewModel->GetHealthSpacing());
	HandleShieldSegmentChanged(ViewModel->GetShieldNumSegments(), ViewModel->GetShieldSpacing());
	HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPREnemyStatusWidget::HandleVisibilityChanged(bool bVisible)
{
	// 위젯 가시성 적용
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPREnemyStatusWidget::HandleHealthChanged(float Current, float Max)
{
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(HealthBar, Percent);
	}
}

void UPREnemyStatusWidget::HandleShieldChanged(float Current, float Max)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(ShieldBar, Percent);
		ShieldBar->SetVisibility(Max > KINDA_SMALL_NUMBER ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPREnemyStatusWidget::HandleHealthSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(HealthBar, NumSegments, Spacing);
	}
}

void UPREnemyStatusWidget::HandleShieldSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(ShieldBar, NumSegments, Spacing);
	}
}
