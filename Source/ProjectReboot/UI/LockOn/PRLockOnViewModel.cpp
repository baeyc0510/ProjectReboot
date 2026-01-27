// PRLockOnViewModel.cpp
#include "PRLockOnViewModel.h"

void UPRLockOnViewModel::InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForActor(InTargetActor, InLocalPlayer);

	// 초기 상태
	bIsVisible = false;
	Progress = 0.0f;
	bIsLocked = false;
}

void UPRLockOnViewModel::Deinitialize()
{
	bIsVisible = false;
	Progress = 0.0f;
	bIsLocked = false;

	Super::Deinitialize();
}

void UPRLockOnViewModel::SetVisible(bool bNewVisible)
{
	if (bIsVisible != bNewVisible)
	{
		bIsVisible = bNewVisible;
		OnVisibilityChanged.Broadcast(bIsVisible);
	}
}

void UPRLockOnViewModel::SetProgress(float NewProgress)
{
	NewProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);

	if (!FMath::IsNearlyEqual(Progress, NewProgress))
	{
		Progress = NewProgress;
		OnProgressChanged.Broadcast(Progress);
	}
}

void UPRLockOnViewModel::SetLocked(bool bNewIsLocked)
{
	if (bIsLocked != bNewIsLocked)
	{
		bIsLocked = bNewIsLocked;
		OnLockedStateChanged.Broadcast(bIsLocked);

		// 락온 완료 시 진행률을 1로 고정
		if (bIsLocked && Progress < 1.0f)
		{
			SetProgress(1.0f);
		}
	}
}
