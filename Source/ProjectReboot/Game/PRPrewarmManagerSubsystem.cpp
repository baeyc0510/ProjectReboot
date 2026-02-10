// Fill out your copyright notice in the Description page of Project Settings.

#include "PRPrewarmManagerSubsystem.h"
#include "PRPrewarmInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/AssetManager.h"

void UPRPrewarmManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPRPrewarmManagerSubsystem::Deinitialize()
{
	PrewarmedAssets.Empty();
	PendingPrewarmAssets.Empty();
	PrewarmHandle.Reset();

	Super::Deinitialize();
}

UPRPrewarmManagerSubsystem* UPRPrewarmManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	return World->GetSubsystem<UPRPrewarmManagerSubsystem>();
}

bool UPRPrewarmManagerSubsystem::TryPrewarmNiagaraSystem(UNiagaraSystem* System)
{
	if (!IsValid(System))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	const FSoftObjectPath AssetPath(System);
	if (!AssetPath.IsValid())
	{
		return false;
	}

	if (PrewarmedAssets.Contains(AssetPath))
	{
		return false;
	}

	UNiagaraComponent* WarmupComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		System,
		PrewarmLocation,
		FRotator::ZeroRotator,
		PrewarmScale,
		true,
		true
	);
	if (!IsValid(WarmupComp))
	{
		return false;
	}

	PrewarmedAssets.Add(AssetPath);
	WarmupComp->Deactivate();
	UE_LOG(LogTemp, Log, TEXT("UPRPrewarmManagerSubsystem: Prewarm Niagara %s"), *AssetPath.ToString());

	return true;
}

void UPRPrewarmManagerSubsystem::ExecutePrewarm(const TArray<UObject*>& RootObjects, bool bLoadSynchronously)
{
	TSet<const UObject*> Visited;
	TSet<FSoftObjectPath> AssetPaths;

	for (UObject* RootObject : RootObjects)
	{
		CollectPrewarmFromObject(RootObject, Visited, AssetPaths);
	}

	PendingPrewarmAssets = AssetPaths.Array();
	if (PendingPrewarmAssets.IsEmpty())
	{
		OnPrewarmComplete.Broadcast();
		return;
	}

	if (bLoadSynchronously)
	{
		for (const FSoftObjectPath& AssetPath : PendingPrewarmAssets)
		{
			UNiagaraSystem* System = Cast<UNiagaraSystem>(AssetPath.TryLoad());
			TryPrewarmNiagaraSystem(System);
		}

		PendingPrewarmAssets.Reset();
		OnPrewarmComplete.Broadcast();
		return;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	PrewarmHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(
		PendingPrewarmAssets,
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandleAsyncPrewarmLoaded)
	);
}

void UPRPrewarmManagerSubsystem::CollectPrewarmFromObject(UObject* RootObject, TSet<const UObject*>& Visited, TSet<FSoftObjectPath>& OutAssets) const
{
	if (!IsValid(RootObject))
	{
		return;
	}

	UObject* TargetObject = RootObject;
	if (UClass* TargetClass = Cast<UClass>(RootObject))
	{
		if (Visited.Contains(TargetClass))
		{
			return;
		}
		Visited.Add(TargetClass);

		TargetObject = TargetClass->GetDefaultObject();
		if (!IsValid(TargetObject) || Visited.Contains(TargetObject))
		{
			return;
		}
		Visited.Add(TargetObject);
	}
	else
	{
		if (Visited.Contains(RootObject))
		{
			return;
		}
		Visited.Add(RootObject);
	}

	IPRPrewarmInterface* PrewarmInterface = Cast<IPRPrewarmInterface>(TargetObject);
	if (!PrewarmInterface)
	{
		return;
	}

	TArray<TSoftObjectPtr<UNiagaraSystem>> Assets;
	PrewarmInterface->GetPrewarmNiagaraAssets(Assets);
	for (const TSoftObjectPtr<UNiagaraSystem>& Asset : Assets)
	{
		if (!Asset.IsNull())
		{
			OutAssets.Add(Asset.ToSoftObjectPath());
		}
	}

	TArray<UObject*> Children;
	PrewarmInterface->GetPrewarmChildren(Children);
	for (UObject* Child : Children)
	{
		CollectPrewarmFromObject(Child, Visited, OutAssets);
	}
}

void UPRPrewarmManagerSubsystem::HandleAsyncPrewarmLoaded()
{
	for (const FSoftObjectPath& AssetPath : PendingPrewarmAssets)
	{
		UNiagaraSystem* System = Cast<UNiagaraSystem>(AssetPath.ResolveObject());
		TryPrewarmNiagaraSystem(System);
	}

	PendingPrewarmAssets.Reset();
	PrewarmHandle.Reset();
	OnPrewarmComplete.Broadcast();
}

void UPRPrewarmManagerSubsystem::ResetPrewarmHistory()
{
	PrewarmedAssets.Empty();
}
