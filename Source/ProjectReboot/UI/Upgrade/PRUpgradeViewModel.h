// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PRUpgradeViewModel.generated.h"

class UPRUpgradeManagerComponent;
class UPRUpgradeModuleData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeViewModelUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeSelected, UPRUpgradeModuleData*, SelectedModule);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradePurchaseResult, bool, bSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeCurrencyUpdated, float, NewCurrency);

/**
 * 업그레이드 UI용 ViewModel
 * MVVM 패턴으로 UI와 비즈니스 로직 분리
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRUpgradeViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	/*~ UPRViewModelBase Interface ~*/
	virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer) override;
	virtual void Deinitialize() override;

	/*~ Initialization ~*/

	// UpgradeManager 바인딩
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ViewModel")
	void BindToManager(UPRUpgradeManagerComponent* InManager);

	// UpgradeManager 바인딩 해제
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ViewModel")
	void UnbindFromManager();

	// 표시할 업그레이드 목록 설정
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ViewModel")
	void SetAvailableUpgrades(const TArray<UPRUpgradeModuleData*>& InUpgrades);

	/*~ Selection & Purchase ~*/

	// 업그레이드 선택
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ViewModel")
	void SelectUpgrade(UPRUpgradeModuleData* InModule);

	// 구매 요청
	UFUNCTION(BlueprintCallable, Category = "Upgrade|ViewModel")
	void RequestPurchase();

	/*~ Property Getters ~*/

	// 사용 가능한 업그레이드 목록 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	const TArray<UPRUpgradeModuleData*>& GetAvailableUpgrades() const { return AvailableUpgrades; }

	// 현재 선택된 업그레이드 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	UPRUpgradeModuleData* GetSelectedUpgrade() const { return SelectedUpgrade; }

	// 현재 레벨 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	// 최대 레벨 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	int32 GetMaxLevel() const { return MaxLevel; }

	// 다음 레벨 비용 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	float GetNextLevelCost() const { return NextLevelCost; }

	// 지정 모듈의 현재 레벨 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	int32 GetModuleCurrentLevel(UPRUpgradeModuleData* InModule) const;

	// 지정 모듈의 최대 레벨 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	int32 GetModuleMaxLevel(UPRUpgradeModuleData* InModule) const;

	// 지정 모듈의 다음 레벨 비용 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	float GetModuleNextLevelCost(UPRUpgradeModuleData* InModule) const;

	// 지정 모듈의 구매 가능 여부 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	bool CanPurchaseModule(UPRUpgradeModuleData* InModule) const;

	// 구매 가능 여부 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	bool CanPurchase() const { return bCanPurchase; }

	// 보유 화폐 반환
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	float GetCurrentCurrency() const { return CurrentCurrency; }

	// 최대 레벨 도달 여부
	UFUNCTION(BlueprintPure, Category = "Upgrade|ViewModel")
	bool IsAtMaxLevel() const { return CurrentLevel >= MaxLevel; }

public:
	/*~ Delegates ~*/

	// ViewModel 상태 갱신 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeViewModelUpdated OnViewModelUpdated;

	// 업그레이드 선택 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeSelected OnUpgradeSelected;

	// 구매 결과 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradePurchaseResult OnPurchaseResult;

	// 화폐 갱신 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeCurrencyUpdated OnCurrencyUpdated;

private:
	// 선택된 업그레이드 정보 갱신
	void RefreshSelectedUpgradeInfo();

	// 화폐 정보 갱신
	void RefreshCurrency();

	// 이벤트 핸들러
	UFUNCTION()
	void HandleUpgradePurchased(UPRUpgradeModuleData* InModule, int32 NewLevel);

	UFUNCTION()
	void HandleCurrencyChanged(FGameplayTag CurrencyTag, float OldValue, float NewValue);

private:
	UPROPERTY()
	TWeakObjectPtr<UPRUpgradeManagerComponent> BoundManager;

	UPROPERTY()
	TArray<UPRUpgradeModuleData*> AvailableUpgrades;

	UPROPERTY()
	UPRUpgradeModuleData* SelectedUpgrade = nullptr;

	int32 CurrentLevel = 0;
	int32 MaxLevel = 0;
	float NextLevelCost = 0.0f;
	bool bCanPurchase = false;
	float CurrentCurrency = 0.0f;
};
