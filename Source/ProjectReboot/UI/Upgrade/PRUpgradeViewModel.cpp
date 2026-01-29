// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeViewModel.h"
#include "ProjectReboot/Upgrade/PRUpgradeManagerComponent.h"
#include "ProjectReboot/Upgrade/PRUpgradeModuleData.h"

void UPRUpgradeViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
}

void UPRUpgradeViewModel::Deinitialize()
{
	UnbindFromManager();
	AvailableUpgrades.Empty();
	SelectedUpgrade = nullptr;

	Super::Deinitialize();
}

void UPRUpgradeViewModel::BindToManager(UPRUpgradeManagerComponent* InManager)
{
	if (!IsValid(InManager))
	{
		return;
	}

	// 기존 바인딩 해제
	UnbindFromManager();

	BoundManager = InManager;

	// 이벤트 구독
	BoundManager->OnUpgradePurchased.AddDynamic(this, &ThisClass::HandleUpgradePurchased);
	BoundManager->OnCurrencyChanged.AddDynamic(this, &ThisClass::HandleCurrencyChanged);

	// 초기 화폐 정보 갱신
	RefreshCurrency();
}

void UPRUpgradeViewModel::UnbindFromManager()
{
	if (!BoundManager.IsValid())
	{
		return;
	}

	BoundManager->OnUpgradePurchased.RemoveDynamic(this, &ThisClass::HandleUpgradePurchased);
	BoundManager->OnCurrencyChanged.RemoveDynamic(this, &ThisClass::HandleCurrencyChanged);
	BoundManager.Reset();
}

void UPRUpgradeViewModel::SetAvailableUpgrades(const TArray<UPRUpgradeModuleData*>& InUpgrades)
{
	AvailableUpgrades = InUpgrades;

	// 선택된 업그레이드가 새 목록에 없으면 선택 해제
	if (SelectedUpgrade && !AvailableUpgrades.Contains(SelectedUpgrade))
	{
		SelectedUpgrade = nullptr;
		RefreshSelectedUpgradeInfo();
		OnUpgradeSelected.Broadcast(nullptr);
	}

	OnViewModelUpdated.Broadcast();
}

void UPRUpgradeViewModel::SelectUpgrade(UPRUpgradeModuleData* InModule)
{
	if (SelectedUpgrade == InModule)
	{
		return;
	}

	SelectedUpgrade = InModule;
	RefreshSelectedUpgradeInfo();

	OnUpgradeSelected.Broadcast(SelectedUpgrade);
	OnViewModelUpdated.Broadcast();
}

void UPRUpgradeViewModel::RequestPurchase()
{
	if (!BoundManager.IsValid() || !IsValid(SelectedUpgrade))
	{
		OnPurchaseResult.Broadcast(false, TEXT("No upgrade selected"));
		return;
	}

	FString FailReason;
	bool bSuccess = BoundManager->TryPurchaseUpgrade(SelectedUpgrade, FailReason);
	OnPurchaseResult.Broadcast(bSuccess, bSuccess ? TEXT("Success") : FailReason);
}

int32 UPRUpgradeViewModel::GetModuleCurrentLevel(UPRUpgradeModuleData* InModule) const
{
	if (!BoundManager.IsValid() || !IsValid(InModule))
	{
		return 0;
	}

	return BoundManager->GetUpgradeLevel(InModule);
}

int32 UPRUpgradeViewModel::GetModuleMaxLevel(UPRUpgradeModuleData* InModule) const
{
	if (!BoundManager.IsValid() || !IsValid(InModule))
	{
		return 0;
	}

	return BoundManager->GetMaxLevel(InModule);
}

float UPRUpgradeViewModel::GetModuleNextLevelCost(UPRUpgradeModuleData* InModule) const
{
	if (!BoundManager.IsValid() || !IsValid(InModule))
	{
		return 0.0f;
	}

	return BoundManager->GetNextLevelCost(InModule);
}

bool UPRUpgradeViewModel::CanPurchaseModule(UPRUpgradeModuleData* InModule) const
{
	if (!BoundManager.IsValid() || !IsValid(InModule))
	{
		return false;
	}

	return BoundManager->CanPurchaseUpgrade(InModule);
}

void UPRUpgradeViewModel::RefreshSelectedUpgradeInfo()
{
	if (!IsValid(SelectedUpgrade) || !BoundManager.IsValid())
	{
		CurrentLevel = 0;
		MaxLevel = 0;
		NextLevelCost = 0.0f;
		bCanPurchase = false;
		return;
	}

	CurrentLevel = BoundManager->GetUpgradeLevel(SelectedUpgrade);
	MaxLevel = BoundManager->GetMaxLevel(SelectedUpgrade);
	NextLevelCost = BoundManager->GetNextLevelCost(SelectedUpgrade);
	bCanPurchase = BoundManager->CanPurchaseUpgrade(SelectedUpgrade);

	RefreshCurrency();
}

void UPRUpgradeViewModel::RefreshCurrency()
{
	if (!BoundManager.IsValid() || !IsValid(SelectedUpgrade))
	{
		return;
	}

	float NewCurrency = BoundManager->GetCurrency(SelectedUpgrade->CurrencyTag);
	if (CurrentCurrency != NewCurrency)
	{
		CurrentCurrency = NewCurrency;
		OnCurrencyUpdated.Broadcast(CurrentCurrency);
	}
}

void UPRUpgradeViewModel::HandleUpgradePurchased(UPRUpgradeModuleData* InModule, int32 NewLevel)
{
	if (InModule == SelectedUpgrade)
	{
		RefreshSelectedUpgradeInfo();
		OnViewModelUpdated.Broadcast();
	}
}

void UPRUpgradeViewModel::HandleCurrencyChanged(FGameplayTag CurrencyTag, float OldValue, float NewValue)
{
	// 현재 선택된 업그레이드의 화폐 태그와 일치하면 갱신
	if (IsValid(SelectedUpgrade) && SelectedUpgrade->CurrencyTag == CurrencyTag)
	{
		CurrentCurrency = NewValue;
		bCanPurchase = BoundManager.IsValid() && BoundManager->CanPurchaseUpgrade(SelectedUpgrade);

		OnCurrencyUpdated.Broadcast(CurrentCurrency);
		OnViewModelUpdated.Broadcast();
	}
}
