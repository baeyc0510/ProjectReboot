// Fill out your copyright notice in the Description page of Project Settings.

#include "PRUpgradeViewModel.h"
#include "ProjectReboot/Upgrade/PRUpgradeManagerSubsystem.h"
#include "ProjectReboot/Upgrade/PRUpgradeModuleData.h"
#include "ProjectReboot/PRGameplayTags.h"

UPRUpgradeViewModel::UPRUpgradeViewModel()
{
	// ViewModel Tag 설정
	ViewModelTag = TAG_UI_ViewModel_Upgrade;
}

void UPRUpgradeViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
	// 업그레이드 패널 기본 표시
	SetVisible(true);

	BindToSubsystem();
}

void UPRUpgradeViewModel::Deinitialize()
{
	UnbindFromSubsystem();
	AvailableUpgrades.Empty();
	SelectedUpgrade = nullptr;

	Super::Deinitialize();
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
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(SelectedUpgrade))
	{
		OnPurchaseResult.Broadcast(false, TEXT("No upgrade selected"));
		return;
	}

	FString FailReason;
	bool bSuccess = Subsystem->TryPurchaseUpgrade(SelectedUpgrade, FailReason);
	OnPurchaseResult.Broadcast(bSuccess, bSuccess ? TEXT("Success") : FailReason);
}

int32 UPRUpgradeViewModel::GetModuleCurrentLevel(UPRUpgradeModuleData* InModule) const
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(InModule))
	{
		return 0;
	}

	return Subsystem->GetUpgradeLevel(InModule);
}

int32 UPRUpgradeViewModel::GetModuleMaxLevel(UPRUpgradeModuleData* InModule) const
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(InModule))
	{
		return 0;
	}

	return Subsystem->GetMaxLevel(InModule);
}

float UPRUpgradeViewModel::GetModuleNextLevelCost(UPRUpgradeModuleData* InModule) const
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(InModule))
	{
		return 0.0f;
	}

	return Subsystem->GetNextLevelCost(InModule);
}

bool UPRUpgradeViewModel::CanPurchaseModule(UPRUpgradeModuleData* InModule) const
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(InModule))
	{
		return false;
	}

	return Subsystem->CanPurchaseUpgrade(InModule);
}

void UPRUpgradeViewModel::BindToSubsystem()
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnUpgradePurchased.AddDynamic(this, &ThisClass::HandleUpgradePurchased);
	Subsystem->OnCurrencyChanged.AddDynamic(this, &ThisClass::HandleCurrencyChanged);
	bIsBoundToSubsystem = true;

	// 초기 화폐 정보 갱신
	RefreshCurrency();
}

void UPRUpgradeViewModel::UnbindFromSubsystem()
{
	if (!bIsBoundToSubsystem)
	{
		return;
	}

	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->OnUpgradePurchased.RemoveDynamic(this, &ThisClass::HandleUpgradePurchased);
	Subsystem->OnCurrencyChanged.RemoveDynamic(this, &ThisClass::HandleCurrencyChanged);
	bIsBoundToSubsystem = false;
}

UPRUpgradeManagerSubsystem* UPRUpgradeViewModel::GetUpgradeSubsystem() const
{
	return UPRUpgradeManagerSubsystem::Get(this);
}

void UPRUpgradeViewModel::RefreshSelectedUpgradeInfo()
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(SelectedUpgrade) || !IsValid(Subsystem))
	{
		CurrentLevel = 0;
		MaxLevel = 0;
		NextLevelCost = 0.0f;
		bCanPurchase = false;
		return;
	}

	CurrentLevel = Subsystem->GetUpgradeLevel(SelectedUpgrade);
	MaxLevel = Subsystem->GetMaxLevel(SelectedUpgrade);
	NextLevelCost = Subsystem->GetNextLevelCost(SelectedUpgrade);
	bCanPurchase = Subsystem->CanPurchaseUpgrade(SelectedUpgrade);

	RefreshCurrency();
}

void UPRUpgradeViewModel::RefreshCurrency()
{
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();
	if (!IsValid(Subsystem) || !IsValid(SelectedUpgrade))
	{
		return;
	}

	float NewCurrency = Subsystem->GetCurrency(SelectedUpgrade->CurrencyTag);
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
	UPRUpgradeManagerSubsystem* Subsystem = GetUpgradeSubsystem();

	// 현재 선택된 업그레이드의 화폐 태그와 일치하면 갱신
	if (IsValid(SelectedUpgrade) && SelectedUpgrade->CurrencyTag == CurrencyTag)
	{
		CurrentCurrency = NewValue;
		bCanPurchase = IsValid(Subsystem) && Subsystem->CanPurchaseUpgrade(SelectedUpgrade);

		OnCurrencyUpdated.Broadcast(CurrentCurrency);
		OnViewModelUpdated.Broadcast();
	}
}
