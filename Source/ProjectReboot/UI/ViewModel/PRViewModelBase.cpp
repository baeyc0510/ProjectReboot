// PRViewModelBase.cpp
#include "PRViewModelBase.h"
#include "Engine/LocalPlayer.h"

void UPRViewModelBase::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	LocalPlayer = InLocalPlayer;
	TargetActor.Reset();
}

void UPRViewModelBase::InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer)
{
	LocalPlayer = InLocalPlayer;
	TargetActor = InTargetActor;
}

void UPRViewModelBase::Deinitialize()
{
	bIsVisible = false;
	bDesiredVisible = false;
	bVisibilityOverrideActive = false;
	bVisibilityOverrideValue = false;
	LocalPlayer.Reset();
	TargetActor.Reset();
}

void UPRViewModelBase::SetVisible(bool bNewVisible)
{
	// 기본 가시성 갱신
	bDesiredVisible = bNewVisible;

	// 오버라이드 여부에 따라 최종 가시성 결정
	const bool bNewEffective = bVisibilityOverrideActive ? bVisibilityOverrideValue : bDesiredVisible;

	// 가시성 변경 시에만 브로드캐스트
	if (bIsVisible != bNewEffective)
	{
		bIsVisible = bNewEffective;
		OnVisibilityChanged.Broadcast(bIsVisible);
	}
}

void UPRViewModelBase::SetVisibilityOverride(bool bOverrideVisible)
{
	bVisibilityOverrideActive = true;
	bVisibilityOverrideValue = bOverrideVisible;

	// 오버라이드 해제 시 기본 가시성으로 복원
	const bool bNewEffective = bVisibilityOverrideActive ? bVisibilityOverrideValue : bDesiredVisible;
	if (bIsVisible != bNewEffective)
	{
		bIsVisible = bNewEffective;
		OnVisibilityChanged.Broadcast(bIsVisible);
	}
}

void UPRViewModelBase::ClearVisibilityOverride()
{
	bVisibilityOverrideActive = false;

	// 오버라이드 해제 시 기본 가시성으로 복원
	const bool bNewEffective = bDesiredVisible;
	if (bIsVisible != bNewEffective)
	{
		bIsVisible = bNewEffective;
		OnVisibilityChanged.Broadcast(bIsVisible);
	}
}

APlayerController* UPRViewModelBase::GetOwningPlayerController() const
{
	if (LocalPlayer.IsValid())
	{
		return LocalPlayer->GetPlayerController(LocalPlayer->GetWorld());
	}
	return nullptr;
}