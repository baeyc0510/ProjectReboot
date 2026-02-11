// Fill out your copyright notice in the Description page of Project Settings.

#include "PRPrewarmManagerSubsystem.h"
#include "PRPrewarmInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/AssetManager.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

void UPRPrewarmManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPRPrewarmManagerSubsystem::Deinitialize()
{
	PrewarmedAssets.Empty();
	PendingNiagaraAssets.Empty();
	PendingSoundAssets.Empty();
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

bool UPRPrewarmManagerSubsystem::TryPrewarmSound(USoundBase* Sound)
{
	if (!IsValid(Sound))
	{
		return false;
	}

	const FSoftObjectPath AssetPath(Sound);
	if (!AssetPath.IsValid())
	{
		return false;
	}

	if (PrewarmedAssets.Contains(AssetPath))
	{
		return false;
	}

	// 무음으로 재생하여 사운드 데이터 로딩
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	UGameplayStatics::PlaySound2D(World, Sound, 0.0f);

	PrewarmedAssets.Add(AssetPath);
	UE_LOG(LogTemp, Log, TEXT("UPRPrewarmManagerSubsystem: Prewarm Sound %s"), *AssetPath.ToString());

	return true;
}

void UPRPrewarmManagerSubsystem::ExecutePrewarm(const TArray<UObject*>& RootObjects, bool bLoadSynchronously)
{
	TSet<const UObject*> Visited;
	TSet<FSoftObjectPath> NiagaraPaths;
	TSet<FSoftObjectPath> SoundPaths;

	for (UObject* RootObject : RootObjects)
	{
		CollectPrewarmFromObject(RootObject, Visited, NiagaraPaths, SoundPaths);
	}

	PendingNiagaraAssets = NiagaraPaths.Array();
	PendingSoundAssets = SoundPaths.Array();

	if (PendingNiagaraAssets.IsEmpty() && PendingSoundAssets.IsEmpty())
	{
		OnPrewarmComplete.Broadcast();
		return;
	}

	if (bLoadSynchronously)
	{
		for (const FSoftObjectPath& AssetPath : PendingNiagaraAssets)
		{
			UNiagaraSystem* System = Cast<UNiagaraSystem>(AssetPath.TryLoad());
			TryPrewarmNiagaraSystem(System);
		}

		for (const FSoftObjectPath& AssetPath : PendingSoundAssets)
		{
			USoundBase* Sound = Cast<USoundBase>(AssetPath.TryLoad());
			TryPrewarmSound(Sound);
		}

		PendingNiagaraAssets.Reset();
		PendingSoundAssets.Reset();
		OnPrewarmComplete.Broadcast();
		return;
	}

	TArray<FSoftObjectPath> AllAssets;
	AllAssets.Append(PendingNiagaraAssets);
	AllAssets.Append(PendingSoundAssets);

	UAssetManager& AssetManager = UAssetManager::Get();
	PrewarmHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(
		AllAssets,
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandleAsyncPrewarmLoaded)
	);
}

void UPRPrewarmManagerSubsystem::CollectPrewarmFromObject(UObject* RootObject, TSet<const UObject*>& Visited, TSet<FSoftObjectPath>& OutNiagaraAssets, TSet<FSoftObjectPath>& OutSoundAssets) const
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

	TArray<TSoftObjectPtr<UNiagaraSystem>> NiagaraAssets;
	PrewarmInterface->GetPrewarmNiagaraAssets(NiagaraAssets);
	for (const TSoftObjectPtr<UNiagaraSystem>& Asset : NiagaraAssets)
	{
		if (!Asset.IsNull())
		{
			OutNiagaraAssets.Add(Asset.ToSoftObjectPath());
		}
	}

	TArray<TSoftObjectPtr<USoundBase>> SoundAssets;
	PrewarmInterface->GetPrewarmSoundAssets(SoundAssets);
	for (const TSoftObjectPtr<USoundBase>& Asset : SoundAssets)
	{
		if (!Asset.IsNull())
		{
			OutSoundAssets.Add(Asset.ToSoftObjectPath());
		}
	}

	TArray<UObject*> Children;
	PrewarmInterface->GetPrewarmChildren(Children);
	for (UObject* Child : Children)
	{
		CollectPrewarmFromObject(Child, Visited, OutNiagaraAssets, OutSoundAssets);
	}
}

void UPRPrewarmManagerSubsystem::HandleAsyncPrewarmLoaded()
{
	for (const FSoftObjectPath& AssetPath : PendingNiagaraAssets)
	{
		UNiagaraSystem* System = Cast<UNiagaraSystem>(AssetPath.ResolveObject());
		TryPrewarmNiagaraSystem(System);
	}

	for (const FSoftObjectPath& AssetPath : PendingSoundAssets)
	{
		USoundBase* Sound = Cast<USoundBase>(AssetPath.ResolveObject());
		TryPrewarmSound(Sound);
	}

	PendingNiagaraAssets.Reset();
	PendingSoundAssets.Reset();
	PrewarmHandle.Reset();
	OnPrewarmComplete.Broadcast();
}

void UPRPrewarmManagerSubsystem::ResetPrewarmHistory()
{
	PrewarmedAssets.Empty();
}
