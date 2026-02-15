// PRAT_FireMissilesSequentially.cpp
#include "PRAT_FireMissilesSequentially.h"
#include "AbilitySystemComponent.h"

UPRAT_FireMissilesSequentially* UPRAT_FireMissilesSequentially::CreateTask(
	UGameplayAbility* OwningAbility,
	const TArray<AActor*>& InTargets,
	float InFireInterval)
{
	UPRAT_FireMissilesSequentially* Task = NewAbilityTask<UPRAT_FireMissilesSequentially>(OwningAbility);
	Task->FireInterval = InFireInterval;
	Task->AccumulatedTime = 0.0f;
	Task->CurrentIndex = 0;

	Task->Targets.Reserve(InTargets.Num());
	for (AActor* Target : InTargets)
	{
		Task->Targets.Add(Target);
	}

	return Task;
}

void UPRAT_FireMissilesSequentially::Activate()
{
	Super::Activate();

	bTickingTask = true;

	// 첫 발 즉시 발사
	if (Targets.IsValidIndex(CurrentIndex))
	{
		OnFireNextMissile.Broadcast(Targets[CurrentIndex].Get());
		CurrentIndex++;
	}

	// 타겟이 1개 이하면 즉시 완료
	if (CurrentIndex >= Targets.Num())
	{
		OnAllMissilesFired.Broadcast();
		EndTask();
	}
}

void UPRAT_FireMissilesSequentially::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	// AvatarActor의 CustomTimeDilation 반영
	AActor* AvatarActor = AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetAvatarActor()
		: nullptr;

	float ActorTimeDilation = IsValid(AvatarActor) ? AvatarActor->CustomTimeDilation : 1.0f;
	AccumulatedTime += DeltaTime * ActorTimeDilation;

	if (AccumulatedTime >= FireInterval)
	{
		AccumulatedTime -= FireInterval;

		if (Targets.IsValidIndex(CurrentIndex))
		{
			OnFireNextMissile.Broadcast(Targets[CurrentIndex].Get());
			CurrentIndex++;
		}

		if (CurrentIndex >= Targets.Num())
		{
			OnAllMissilesFired.Broadcast();
			EndTask();
		}
	}
}
