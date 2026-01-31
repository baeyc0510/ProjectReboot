// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionPromptWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "ProjectReboot/UI/Interaction/PRInteractionViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

void UPRInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ViewModel 바인딩 후 초기 상태 반영
	BindToViewModel();
	RefreshFromViewModel();
}

void UPRInteractionPromptWidget::NativeDestruct()
{
	// 바인딩 해제
	UnbindFromViewModel();

	Super::NativeDestruct();
}

void UPRInteractionPromptWidget::BindToViewModel()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UPRViewModelSubsystem* ViewModelSubsystem = LocalPlayer->GetSubsystem<UPRViewModelSubsystem>();
	if (!IsValid(ViewModelSubsystem))
	{
		return;
	}

	UPRInteractionViewModel* ViewModel = ViewModelSubsystem->GetOrCreateGlobalViewModel<UPRInteractionViewModel>();
	if (!IsValid(ViewModel))
	{
		return;
	}

	BoundViewModel = ViewModel;
	BoundViewModel->OnViewModelUpdated.AddDynamic(this, &ThisClass::HandleViewModelUpdated);
	BoundViewModel->OnVisibilityChanged.AddDynamic(this, &ThisClass::HandleVisibilityChanged);
	BoundViewModel->OnEnabledChanged.AddDynamic(this, &ThisClass::HandleEnabledChanged);
	BoundViewModel->OnContentChanged.AddDynamic(this, &ThisClass::HandleContentChanged);
}

void UPRInteractionPromptWidget::UnbindFromViewModel()
{
	if (BoundViewModel.IsValid())
	{
		BoundViewModel->OnViewModelUpdated.RemoveDynamic(this, &ThisClass::HandleViewModelUpdated);
		BoundViewModel->OnVisibilityChanged.RemoveDynamic(this, &ThisClass::HandleVisibilityChanged);
		BoundViewModel->OnEnabledChanged.RemoveDynamic(this, &ThisClass::HandleEnabledChanged);
		BoundViewModel->OnContentChanged.RemoveDynamic(this, &ThisClass::HandleContentChanged);
	}

	BoundViewModel.Reset();
}

void UPRInteractionPromptWidget::HandleVisibilityChanged(bool bVisible)
{
	// 표시 여부 적용
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UPRInteractionPromptWidget::HandleEnabledChanged(bool bEnabled)
{
	// 활성 상태 적용
	SetIsEnabled(bEnabled);
	SetRenderOpacity(bEnabled ? 1.0f : 0.4f);
}

void UPRInteractionPromptWidget::HandleContentChanged(const FText& InText, UTexture2D* InIcon)
{
	// 텍스트 갱신
	if (InteractionText)
	{
		InteractionText->SetText(InText);
	}

	// 아이콘 갱신
	if (InteractionIcon)
	{
		InteractionIcon->SetBrushFromTexture(InIcon);
		InteractionIcon->SetVisibility(InIcon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPRInteractionPromptWidget::HandleViewModelUpdated()
{
	// 전체 상태 갱신
	RefreshFromViewModel();
}

void UPRInteractionPromptWidget::RefreshFromViewModel()
{
	if (!BoundViewModel.IsValid())
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	HandleVisibilityChanged(BoundViewModel->IsVisible());
	HandleEnabledChanged(BoundViewModel->IsEnabled());
	HandleContentChanged(BoundViewModel->GetDisplayText(), BoundViewModel->GetIcon());
}