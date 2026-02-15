// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeManagerSubsystem.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "PRUpgradeModuleData.h"
#include "ProjectReboot/PRGameplayTags.h"

void UPRUpgradeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// RogueliteSubsystem 의존성 추가
	Collection.InitializeDependency<URogueliteSubsystem>();

	BindToRogueliteSubsystem();
}

void UPRUpgradeManagerSubsystem::Deinitialize()
{
	UnbindFromRogueliteSubsystem();
	PurchasedModules.Empty();

	Super::Deinitialize();
}

UPRUpgradeManagerSubsystem* UPRUpgradeManagerSubsystem::Get(const UObject* WorldContextObject)
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

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPRUpgradeManagerSubsystem>();
}

bool UPRUpgradeManagerSubsystem::TryPurchaseUpgrade(UPRUpgradeModuleData* InModule, FString& OutFailReason)
{
	if (!IsValid(InModule))
	{
		OutFailReason = TEXT("Invalid module");
		return false;
	}

	URogueliteActionData* ActionData = InModule->GetActionData();
	if (!IsValid(ActionData))
	{
		OutFailReason = TEXT("Invalid action data");
		return false;
	}

	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem))
	{
		OutFailReason = TEXT("Roguelite system not available");
		return false;
	}

	// 현재 레벨 조회
	const int32 CurrentLevel = GetUpgradeLevel(InModule);
	const int32 MaxLevel = GetMaxLevel(InModule);

	// 최대 레벨 검증
	if (CurrentLevel >= MaxLevel)
	{
		OutFailReason = TEXT("Already at max level");
		return false;
	}

	// 비용 계산 및 화폐 검증
	const float Cost = InModule->GetCostForLevel(CurrentLevel + 1);
	const float Currency = GetCurrency(InModule->CurrencyTag);

	if (Currency < Cost)
	{
		OutFailReason = FString::Printf(TEXT("Not enough currency (need %.0f, have %.0f)"), Cost, Currency);
		return false;
	}

	// 화폐 차감
	Subsystem->AddStateValue(InModule->CurrencyTag, -Cost);

	// 업그레이드 획득 (RogueliteSubsystem에 위임)
	FString AcquireFailReason;
	if (!Subsystem->TryAcquireAction(ActionData, AcquireFailReason))
	{
		// 롤백: 화폐 복구
		Subsystem->AddStateValue(InModule->CurrencyTag, Cost);
		OutFailReason = AcquireFailReason;
		return false;
	}

	// 구매 기록 추가/갱신
	const int32 NewLevel = CurrentLevel + 1;
	FPRUpgradePurchaseInfo* ExistingInfo = PurchasedModules.FindByPredicate(
		[InModule](const FPRUpgradePurchaseInfo& Info)
		{
			return Info.Module.Get() == InModule;
		});

	if (ExistingInfo)
	{
		ExistingInfo->PurchasedLevel = NewLevel;
	}
	else
	{
		FPRUpgradePurchaseInfo NewInfo;
		NewInfo.Module = InModule;
		NewInfo.PurchasedLevel = NewLevel;
		PurchasedModules.Add(NewInfo);
	}

	// 성공 이벤트 브로드캐스트
	OnUpgradePurchased.Broadcast(InModule, NewLevel);

	return true;
}

bool UPRUpgradeManagerSubsystem::CanPurchaseUpgrade(UPRUpgradeModuleData* InModule) const
{
	if (!IsValid(InModule))
	{
		return false;
	}

	if (!IsValid(InModule->GetActionData()))
	{
		return false;
	}

	const int32 CurrentLevel = GetUpgradeLevel(InModule);
	const int32 MaxLevel = GetMaxLevel(InModule);

	// 최대 레벨 도달 시 구매 불가
	if (CurrentLevel >= MaxLevel)
	{
		return false;
	}

	// 화폐 검증
	const float Cost = InModule->GetCostForLevel(CurrentLevel + 1);
	const float Currency = GetCurrency(InModule->CurrencyTag);

	return Currency >= Cost;
}

int32 UPRUpgradeManagerSubsystem::GetUpgradeLevel(UPRUpgradeModuleData* InModule) const
{
	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem) || !IsValid(InModule))
	{
		return 0;
	}

	URogueliteActionData* ActionData = InModule->GetActionData();
	if (!IsValid(ActionData))
	{
		return 0;
	}

	return Subsystem->GetActionStacks(ActionData);
}

int32 UPRUpgradeManagerSubsystem::GetMaxLevel(UPRUpgradeModuleData* InModule) const
{
	if (!IsValid(InModule))
	{
		return 0;
	}
	return InModule->GetMaxStacks();
}

float UPRUpgradeManagerSubsystem::GetNextLevelCost(UPRUpgradeModuleData* InModule) const
{
	if (!IsValid(InModule))
	{
		return 0.0f;
	}

	const int32 CurrentLevel = GetUpgradeLevel(InModule);
	return InModule->GetCostForLevel(CurrentLevel + 1);
}

float UPRUpgradeManagerSubsystem::GetCurrency(FGameplayTag CurrencyTag) const
{
	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem))
	{
		return 0.0f;
	}
	return Subsystem->GetStateValue(CurrencyTag);
}

const TArray<FPRUpgradePurchaseInfo>& UPRUpgradeManagerSubsystem::GetPurchasedModules() const
{
	return PurchasedModules;
}

void UPRUpgradeManagerSubsystem::BindToRogueliteSubsystem()
{
	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnRogueliteStateValueChanged.AddDynamic(this, &ThisClass::HandleRunStateValueChanged);
	bIsBoundToSubsystem = true;
}

void UPRUpgradeManagerSubsystem::UnbindFromRogueliteSubsystem()
{
	if (!bIsBoundToSubsystem)
	{
		return;
	}

	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnRogueliteStateValueChanged.RemoveDynamic(this, &ThisClass::HandleRunStateValueChanged);
	bIsBoundToSubsystem = false;
}

URogueliteSubsystem* UPRUpgradeManagerSubsystem::GetRogueliteSubsystem() const
{
	return URogueliteSubsystem::Get(this);
}

void UPRUpgradeManagerSubsystem::HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue)
{
	// Currency 태그인지 확인하고 이벤트 브로드캐스트
	if (Key.MatchesTag(TAG_Currency) || Key.MatchesTag(TAG_MetaState_Currency))
	{
		OnCurrencyChanged.Broadcast(Key, OldValue, NewValue);
	}
}
