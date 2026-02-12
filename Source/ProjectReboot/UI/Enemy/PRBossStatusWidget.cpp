// PRBossStatusWidget.cpp
#include "PRBossStatusWidget.h"

#include "PREnemyStatusViewModel.h"
#include "ProjectReboot/UI/Interfaces/PRProgressBarInterface.h"
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
	ViewModel->OnHealthSegmentChanged.AddDynamic(this, &UPRBossStatusWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.AddDynamic(this, &UPRBossStatusWidget::HandleShieldSegmentChanged);
	ViewModel->OnDestructStatus.AddDynamic(this, &UPRBossStatusWidget::HandleDestructStatus);
	ViewModel->OnVisibilityChanged.AddDynamic(this, &UPRBossStatusWidget::HandleVisibilityChanged);

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
	ViewModel->OnHealthSegmentChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleShieldSegmentChanged);
	ViewModel->OnDestructStatus.RemoveDynamic(this, &UPRBossStatusWidget::HandleDestructStatus);
	ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPRBossStatusWidget::HandleVisibilityChanged);

	ViewModel = nullptr;
}

void UPRBossStatusWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	// 초기 상태 동기화
	HandleEnemyDisplayNameChanged(ViewModel->GetEnemyDisplayName());
	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
	HandleHealthSegmentChanged(ViewModel->GetHealthNumSegments(), ViewModel->GetHealthSpacing());
	HandleShieldSegmentChanged(ViewModel->GetShieldNumSegments(), ViewModel->GetShieldSpacing());
	HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPRBossStatusWidget::HandleVisibilityChanged(bool bVisible)
{
	// 위젯 가시성 적용
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(HealthBar, Percent);
	}
}

void UPRBossStatusWidget::HandleShieldChanged(float Current, float Max)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(ShieldBar, Percent);
		ShieldBar->SetVisibility(Max > KINDA_SMALL_NUMBER ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPRBossStatusWidget::HandleHealthSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(HealthBar, NumSegments, Spacing);
	}
}

void UPRBossStatusWidget::HandleShieldSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(ShieldBar, NumSegments, Spacing);
	}
}

void UPRBossStatusWidget::HandleDestructStatus()
{
	if (IsInViewport())
	{
		RemoveFromParent();
		Destruct();
	}
}
