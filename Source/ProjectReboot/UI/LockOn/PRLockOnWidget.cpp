// PRLockOnWidget.cpp
#include "PRLockOnWidget.h"

#include "PRLockOnViewModel.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UPRLockOnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Material Instance Dynamic 생성
	if (LockOnImage && LockOnMaterial)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(LockOnMaterial, this);
		LockOnImage->SetBrushFromMaterial(MaterialInstance);
	}
}

void UPRLockOnWidget::NativeDestruct()
{
	UnbindViewModel();
	Super::NativeDestruct();
}

void UPRLockOnWidget::BindViewModel(UPRLockOnViewModel* InViewModel)
{
	UnbindViewModel();

	ViewModel = InViewModel;
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnVisibilityChanged.AddDynamic(this, &UPRLockOnWidget::HandleVisibilityChanged);
	ViewModel->OnProgressChanged.AddDynamic(this, &UPRLockOnWidget::HandleProgressChanged);
	ViewModel->OnLockedStateChanged.AddDynamic(this, &UPRLockOnWidget::HandleLockedStateChanged);

	ApplyInitialState();
}

void UPRLockOnWidget::UnbindViewModel()
{
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPRLockOnWidget::HandleVisibilityChanged);
	ViewModel->OnProgressChanged.RemoveDynamic(this, &UPRLockOnWidget::HandleProgressChanged);
	ViewModel->OnLockedStateChanged.RemoveDynamic(this, &UPRLockOnWidget::HandleLockedStateChanged);

	ViewModel = nullptr;
}

void UPRLockOnWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	// 초기 가시성 설정
	HandleVisibilityChanged(ViewModel->IsVisible());
	SetProgress(ViewModel->GetProgress());
}

void UPRLockOnWidget::SetProgress(float Progress)
{
	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue(ProgressParameterName, Progress);
	}
}

void UPRLockOnWidget::HandleVisibilityChanged(bool bIsVisible)
{
	SetVisibility(bIsVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (bIsVisible)
	{
		SetProgress(0.0f);
	}
}

void UPRLockOnWidget::HandleProgressChanged(float Progress)
{
	SetProgress(Progress);
}

void UPRLockOnWidget::HandleLockedStateChanged(bool bIsLocked)
{
	if (bIsLocked)
	{
		SetProgress(1.0f);
	}
}
