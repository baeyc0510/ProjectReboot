// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionViewModel.h"

#include "ProjectReboot/PRGameplayTags.h"

UPRInteractionViewModel::UPRInteractionViewModel()
{
	// ViewModel Tag 설정
	ViewModelTag = TAG_UI_ViewModel_Interaction;
}

void UPRInteractionViewModel::SetInteractionInfo(const FPRInteractionInfo& InInfo, bool bVisible)
{
	// 변경 전 상태 보관
	const bool bPrevEnabled = bIsEnabled;
	const FText PrevText = DisplayText;
	UTexture2D* PrevIcon = Icon;

	SetVisible(bVisible);
	DisplayText = InInfo.DisplayText;
	Icon = InInfo.Icon;
	bIsEnabled = InInfo.bIsEnabled;

	if (bPrevEnabled != bIsEnabled)
	{
		OnEnabledChanged.Broadcast(bIsEnabled);
	}

	if (!PrevText.EqualTo(DisplayText) || PrevIcon != Icon)
	{
		OnContentChanged.Broadcast(DisplayText, Icon);
	}

	OnViewModelUpdated.Broadcast();
}

void UPRInteractionViewModel::ClearInteractionInfo()
{
	// 변경 전 상태 보관
	const bool bPrevEnabled = bIsEnabled;
	const FText PrevText = DisplayText;
	UTexture2D* PrevIcon = Icon;

	SetVisible(false);
	DisplayText = FText::GetEmpty();
	Icon = nullptr;
	bIsEnabled = false;

	if (bPrevEnabled != bIsEnabled)
	{
		OnEnabledChanged.Broadcast(bIsEnabled);
	}

	if (!PrevText.EqualTo(DisplayText) || PrevIcon != Icon)
	{
		OnContentChanged.Broadcast(DisplayText, Icon);
	}

	OnViewModelUpdated.Broadcast();
}