// Fill out your copyright notice in the Description page of Project Settings.

#include "PRInteractionViewModel.h"

void UPRInteractionViewModel::SetInteractionInfo(const FPRInteractionInfo& InInfo, bool bVisible)
{
	// 변경 전 상태 보관
	const bool bPrevVisible = bIsVisible;
	const bool bPrevEnabled = bIsEnabled;
	const FText PrevText = DisplayText;
	UTexture2D* PrevIcon = Icon;

	bIsVisible = bVisible;
	DisplayText = InInfo.DisplayText;
	Icon = InInfo.Icon;
	bIsEnabled = InInfo.bIsEnabled;

	if (bPrevVisible != bIsVisible)
	{
		OnVisibilityChanged.Broadcast(bIsVisible);
	}

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
	const bool bPrevVisible = bIsVisible;
	const bool bPrevEnabled = bIsEnabled;
	const FText PrevText = DisplayText;
	UTexture2D* PrevIcon = Icon;

	bIsVisible = false;
	DisplayText = FText::GetEmpty();
	Icon = nullptr;
	bIsEnabled = false;

	if (bPrevVisible != bIsVisible)
	{
		OnVisibilityChanged.Broadcast(bIsVisible);
	}

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