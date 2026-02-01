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
			UnregisterViewModelTag(Pair.Value);
			Pair.Value->Deinitialize();
		}
	}
	GlobalViewModelMap.Empty();
	GlobalViewModelTagMap.Empty();

	// Actor-Bound ViewModel 정리
	for (auto& Pair : ActorViewModelMap)
	{
		if (IsValid(Pair.Value))
		{
			UnregisterViewModelTag(Pair.Value);
			Pair.Value->Deinitialize();
		}
	}
	ActorViewModelMap.Empty();
	ActorViewModelTagMap.Empty();
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
		RegisterViewModelTag(NewViewModel, true);
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
			UnregisterViewModelTag(Removed);
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
		RegisterViewModelTag(NewViewModel, false);

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
			UnregisterViewModelTag(Removed);
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
				UnregisterViewModelTag(It.Value());
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

// =====================================================
// ViewModel Tag
// =====================================================

UPRViewModelBase* UPRViewModelSubsystem::FindGlobalViewModelByTag(FGameplayTag ViewModelTag) const
{
	// Tag 미유효 시 검색 생략
	if (!ViewModelTag.IsValid())
	{
		return nullptr;
	}

	if (const TWeakObjectPtr<UPRViewModelBase>* Found = GlobalViewModelTagMap.Find(ViewModelTag))
	{
		return Found->Get();
	}

	return nullptr;
}

void UPRViewModelSubsystem::FindActorViewModelsByTag(FGameplayTag ViewModelTag, TArray<UPRViewModelBase*>& OutViewModels) const
{
	// 결과 초기화
	OutViewModels.Reset();

	if (!ViewModelTag.IsValid())
	{
		return;
	}

	if (const FViewModelTagList* Found = ActorViewModelTagMap.Find(ViewModelTag))
	{
		for (const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr : Found->ViewModels)
		{
			if (ViewModelPtr.IsValid())
			{
				OutViewModels.Add(ViewModelPtr.Get());
			}
		}
	}
}

bool UPRViewModelSubsystem::SetVisibilityByTag(FGameplayTag ViewModelTag, bool bNewVisible, bool bAffectActorBound)
{
	bool bHasChanged = false;

	// Tag 미유효 시 처리하지 않음
	if (!ViewModelTag.IsValid())
	{
		return false;
	}

	if (UPRViewModelBase* GlobalViewModel = FindGlobalViewModelByTag(ViewModelTag))
	{
		const bool bPrevVisible = GlobalViewModel->IsVisible();
		GlobalViewModel->SetVisibilityOverride(bNewVisible);
		bHasChanged |= (bPrevVisible != bNewVisible);
	}

	if (bAffectActorBound)
	{
		// Actor-Bound 목록 유효성 정리 후 일괄 적용
		if (FViewModelTagList* Found = ActorViewModelTagMap.Find(ViewModelTag))
		{
			Found->ViewModels.RemoveAll([](const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr)
			{
				return !ViewModelPtr.IsValid();
			});

			for (const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr : Found->ViewModels)
			{
				if (UPRViewModelBase* ViewModel = ViewModelPtr.Get())
				{
					const bool bPrevVisible = ViewModel->IsVisible();
					ViewModel->SetVisibilityOverride(bNewVisible);
					bHasChanged |= (bPrevVisible != bNewVisible);
				}
			}

			if (Found->ViewModels.Num() == 0)
			{
				ActorViewModelTagMap.Remove(ViewModelTag);
			}
		}
	}

	return bHasChanged;
}

bool UPRViewModelSubsystem::GetVisibilityByTag(FGameplayTag ViewModelTag, bool bDefaultVisible) const
{
	// Tag 미유효 시 기본값 반환
	if (!ViewModelTag.IsValid())
	{
		return bDefaultVisible;
	}

	if (UPRViewModelBase* GlobalViewModel = FindGlobalViewModelByTag(ViewModelTag))
	{
		return GlobalViewModel->IsVisible();
	}

	if (const FViewModelTagList* Found = ActorViewModelTagMap.Find(ViewModelTag))
	{
		for (const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr : Found->ViewModels)
		{
			if (ViewModelPtr.IsValid())
			{
				return ViewModelPtr->IsVisible();
			}
		}
	}

	return bDefaultVisible;
}

void UPRViewModelSubsystem::RestoreVisibilityByTag(FGameplayTag ViewModelTag, bool bVisible, bool bAffectActorBound)
{
	if (!ViewModelTag.IsValid())
	{
		return;
	}

	if (UPRViewModelBase* GlobalViewModel = FindGlobalViewModelByTag(ViewModelTag))
	{
		GlobalViewModel->ClearVisibilityOverride();
		GlobalViewModel->SetVisible(bVisible);
	}

	if (bAffectActorBound)
	{
		if (FViewModelTagList* Found = ActorViewModelTagMap.Find(ViewModelTag))
		{
			for (const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr : Found->ViewModels)
			{
				if (UPRViewModelBase* ViewModel = ViewModelPtr.Get())
				{
					ViewModel->ClearVisibilityOverride();
					ViewModel->SetVisible(bVisible);
				}
			}
		}
	}
}

void UPRViewModelSubsystem::RegisterViewModelTag(UPRViewModelBase* ViewModel, bool bIsGlobal)
{
	// 유효하지 않은 ViewModel은 등록하지 않음
	if (!IsValid(ViewModel))
	{
		return;
	}

	const FGameplayTag ViewModelTag = ViewModel->GetViewModelTag();
	if (!ViewModelTag.IsValid())
	{
		return;
	}

	if (bIsGlobal)
	{
		// Global Tag는 1:1 등록
		if (TWeakObjectPtr<UPRViewModelBase>* Found = GlobalViewModelTagMap.Find(ViewModelTag))
		{
			if (Found->IsValid() && Found->Get() != ViewModel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Global ViewModel Tag 중복: %s"), *ViewModelTag.ToString());
			}
			return;
		}

		GlobalViewModelTagMap.Add(ViewModelTag, ViewModel);
		return;
	}

	FViewModelTagList& TagList = ActorViewModelTagMap.FindOrAdd(ViewModelTag);
	// 중복 등록 방지
	const bool bAlreadyExists = TagList.ViewModels.ContainsByPredicate([ViewModel](const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr)
	{
		return ViewModelPtr.Get() == ViewModel;
	});

	if (!bAlreadyExists)
	{
		TagList.ViewModels.Add(ViewModel);
	}
}

void UPRViewModelSubsystem::UnregisterViewModelTag(UPRViewModelBase* ViewModel)
{
	// 유효하지 않은 ViewModel은 해제하지 않음
	if (!IsValid(ViewModel))
	{
		return;
	}

	const FGameplayTag ViewModelTag = ViewModel->GetViewModelTag();
	if (!ViewModelTag.IsValid())
	{
		return;
	}

	if (TWeakObjectPtr<UPRViewModelBase>* Found = GlobalViewModelTagMap.Find(ViewModelTag))
	{
		if (Found->Get() == ViewModel)
		{
			GlobalViewModelTagMap.Remove(ViewModelTag);
		}
	}

	if (FViewModelTagList* TagList = ActorViewModelTagMap.Find(ViewModelTag))
	{
		TagList->ViewModels.RemoveAll([ViewModel](const TWeakObjectPtr<UPRViewModelBase>& ViewModelPtr)
		{
			return !ViewModelPtr.IsValid() || ViewModelPtr.Get() == ViewModel;
		});

		if (TagList->ViewModels.Num() == 0)
		{
			ActorViewModelTagMap.Remove(ViewModelTag);
		}
	}
}
