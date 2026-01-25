// PRViewModelSubsystem.cpp
#include "PRViewModelSubsystem.h"
#include "PRViewModelBase.h"

void UPRViewModelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPRViewModelSubsystem::Deinitialize()
{
	// Global ViewModel 정리
	for (auto& Pair : GlobalViewModelMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Deinitialize();
		}
	}
	GlobalViewModelMap.Empty();

	// Actor-Bound ViewModel 정리
	for (auto& Pair : ActorViewModelMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Deinitialize();
		}
	}
	ActorViewModelMap.Empty();
	BoundActors.Empty();

	Super::Deinitialize();
}

// =====================================================
// Global ViewModel
// =====================================================

UPRViewModelBase* UPRViewModelSubsystem::RegisterGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (!ViewModelClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UPRViewModelBase>* Found = GlobalViewModelMap.Find(ViewModelClass))
	{
		return *Found;
	}

	UPRViewModelBase* NewViewModel = NewObject<UPRViewModelBase>(this, ViewModelClass);
	if (NewViewModel)
	{
		NewViewModel->InitializeForPlayer(GetLocalPlayer());
		GlobalViewModelMap.Add(ViewModelClass, NewViewModel);
	}

	return NewViewModel;
}

UPRViewModelBase* UPRViewModelSubsystem::GetGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass) const
{
	if (const TObjectPtr<UPRViewModelBase>* Found = GlobalViewModelMap.Find(ViewModelClass))
	{
		return *Found;
	}
	return nullptr;
}

UPRViewModelBase* UPRViewModelSubsystem::GetOrCreateGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (UPRViewModelBase* Existing = GetGlobalViewModel(ViewModelClass))
	{
		return Existing;
	}
	return RegisterGlobalViewModel(ViewModelClass);
}

bool UPRViewModelSubsystem::UnregisterGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	TObjectPtr<UPRViewModelBase> Removed;
	if (GlobalViewModelMap.RemoveAndCopyValue(ViewModelClass, Removed))
	{
		if (Removed)
		{
			Removed->Deinitialize();
		}
		return true;
	}
	return false;
}

// =====================================================
// Actor-Bound ViewModel
// =====================================================

UPRViewModelBase* UPRViewModelSubsystem::GetOrCreateActorViewModel(
	AActor* TargetActor,
	TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (!TargetActor || !ViewModelClass)
	{
		return nullptr;
	}

	// 기존 검색
	FActorViewModelKey Key(TargetActor, ViewModelClass);
	if (TObjectPtr<UPRViewModelBase>* Found = ActorViewModelMap.Find(Key))
	{
		return *Found;
	}

	// 새로 생성
	UPRViewModelBase* NewViewModel = NewObject<UPRViewModelBase>(this, ViewModelClass);
	if (NewViewModel)
	{
		NewViewModel->InitializeForActor(TargetActor, GetLocalPlayer());
		ActorViewModelMap.Add(Key, NewViewModel);

		// Actor 파괴 시 자동 정리 (한 번만 바인딩)
		if (!BoundActors.Contains(TargetActor))
		{
			TargetActor->OnDestroyed.AddDynamic(this, &UPRViewModelSubsystem::HandleActorDestroyed);
			BoundActors.Add(TargetActor);
		}
	}

	return NewViewModel;
}

UPRViewModelBase* UPRViewModelSubsystem::FindActorViewModel(
	AActor* TargetActor,
	TSubclassOf<UPRViewModelBase> ViewModelClass) const
{
	if (!TargetActor || !ViewModelClass)
	{
		return nullptr;
	}

	FActorViewModelKey Key(TargetActor, ViewModelClass);
	if (const TObjectPtr<UPRViewModelBase>* Found = ActorViewModelMap.Find(Key))
	{
		return *Found;
	}
	return nullptr;
}

bool UPRViewModelSubsystem::RemoveActorViewModel(
	AActor* TargetActor,
	TSubclassOf<UPRViewModelBase> ViewModelClass)
{
	if (!TargetActor || !ViewModelClass)
	{
		return false;
	}

	FActorViewModelKey Key(TargetActor, ViewModelClass);
	TObjectPtr<UPRViewModelBase> Removed;
	if (ActorViewModelMap.RemoveAndCopyValue(Key, Removed))
	{
		if (Removed)
		{
			Removed->Deinitialize();
		}
		return true;
	}
	return false;
}

void UPRViewModelSubsystem::RemoveAllViewModelsForActor(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	for (auto It = ActorViewModelMap.CreateIterator(); It; ++It)
	{
		if (It.Key().Actor == TargetActor)
		{
			if (It.Value())
			{
				It.Value()->Deinitialize();
			}
			It.RemoveCurrent();
		}
	}

	BoundActors.Remove(TargetActor);
}

void UPRViewModelSubsystem::HandleActorDestroyed(AActor* DestroyedActor)
{
	RemoveAllViewModelsForActor(DestroyedActor);
}
