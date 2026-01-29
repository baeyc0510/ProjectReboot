// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeManagerComponent.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "PRUpgradeModuleData.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRUpgradeManagerComponent::UPRUpgradeManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPRUpgradeManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	BindToRogueliteSubsystem();
}

void UPRUpgradeManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromRogueliteSubsystem();
	Super::EndPlay(EndPlayReason);
}

bool UPRUpgradeManagerComponent::TryPurchaseUpgrade(UPRUpgradeModuleData* InModule, FString& OutFailReason)
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
	Subsystem->AddRunStateValue(InModule->CurrencyTag, -Cost);

	// 업그레이드 획득 (RogueliteSubsystem에 위임)
	FString AcquireFailReason;
	if (!Subsystem->TryAcquireAction(ActionData, AcquireFailReason))
	{
		// 롤백: 화폐 복구
		Subsystem->AddRunStateValue(InModule->CurrencyTag, Cost);
		OutFailReason = AcquireFailReason;
		return false;
	}

	// 성공 이벤트 브로드캐스트
	OnUpgradePurchased.Broadcast(InModule, CurrentLevel + 1);

	return true;
}

bool UPRUpgradeManagerComponent::CanPurchaseUpgrade(UPRUpgradeModuleData* InModule) const
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

int32 UPRUpgradeManagerComponent::GetUpgradeLevel(UPRUpgradeModuleData* InModule) const
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

int32 UPRUpgradeManagerComponent::GetMaxLevel(UPRUpgradeModuleData* InModule) const
{
	if (!IsValid(InModule))
	{
		return 0;
	}
	return InModule->GetMaxStacks();
}

float UPRUpgradeManagerComponent::GetNextLevelCost(UPRUpgradeModuleData* InModule) const
{
	if (!IsValid(InModule))
	{
		return 0.0f;
	}

	const int32 CurrentLevel = GetUpgradeLevel(InModule);
	return InModule->GetCostForLevel(CurrentLevel + 1);
}

float UPRUpgradeManagerComponent::GetCurrency(FGameplayTag CurrencyTag) const
{
	URogueliteSubsystem* Subsystem = GetRogueliteSubsystem();
	if (!IsValid(Subsystem))
	{
		return 0.0f;
	}
	return Subsystem->GetRunStateValue(CurrencyTag);
}

void UPRUpgradeManagerComponent::BindToRogueliteSubsystem()
{
	URogueliteSubsystem* Subsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnRunStateValueChanged.AddDynamic(this, &ThisClass::HandleRunStateValueChanged);
	bIsBoundToSubsystem = true;
}

void UPRUpgradeManagerComponent::UnbindFromRogueliteSubsystem()
{
	if (!bIsBoundToSubsystem)
	{
		return;
	}

	URogueliteSubsystem* Subsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnRunStateValueChanged.RemoveDynamic(this, &ThisClass::HandleRunStateValueChanged);
	bIsBoundToSubsystem = false;
}

URogueliteSubsystem* UPRUpgradeManagerComponent::GetRogueliteSubsystem() const
{
	return URogueliteSubsystem::Get(this);
}

void UPRUpgradeManagerComponent::HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue)
{
	// Currency 태그인지 확인하고 이벤트 브로드캐스트
	if (Key.MatchesTag(TAG_Currency))
	{
		OnCurrencyChanged.Broadcast(Key, OldValue, NewValue);
	}
}
