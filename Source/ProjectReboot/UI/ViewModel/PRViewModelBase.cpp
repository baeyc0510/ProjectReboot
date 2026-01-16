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
	LocalPlayer.Reset();
	TargetActor.Reset();
}

APlayerController* UPRViewModelBase::GetOwningPlayerController() const
{
	if (LocalPlayer.IsValid())
	{
		return LocalPlayer->GetPlayerController(LocalPlayer->GetWorld());
	}
	return nullptr;
}