// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PRUpgradeManagerSubsystem.generated.h"

class URogueliteSubsystem;
class UPRUpgradeModuleData;

/**
 * 업그레이드 구매 정보 (세이브/로드용)
 */
USTRUCT(BlueprintType)
struct FPRUpgradePurchaseInfo
{
	GENERATED_BODY()

	// 구매한 모듈
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	TSoftObjectPtr<UPRUpgradeModuleData> Module;

	// 구매한 레벨
	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	int32 PurchasedLevel = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradePurchased, UPRUpgradeModuleData*, Module, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpgradeCurrencyChanged, FGameplayTag, CurrencyTag, float, OldValue, float, NewValue);

/**
 * 업그레이드 상태 관리 및 구매 로직을 담당하는 서브시스템
 */
UCLASS()
class PROJECTREBOOT_API UPRUpgradeManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/*~ Static Access ~*/

	// 서브시스템 인스턴스 획득
	UFUNCTION(BlueprintCallable, Category = "Upgrade", meta = (WorldContext = "WorldContextObject"))
	static UPRUpgradeManagerSubsystem* Get(const UObject* WorldContextObject);

	/*~ Purchase Operations ~*/

	// 업그레이드 구매 시도
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	bool TryPurchaseUpgrade(UPRUpgradeModuleData* InModule, FString& OutFailReason);

	// 구매 가능 여부 확인
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	bool CanPurchaseUpgrade(UPRUpgradeModuleData* InModule) const;

	/*~ Level Queries ~*/

	// 모듈의 현재 레벨 조회
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetUpgradeLevel(UPRUpgradeModuleData* InModule) const;

	// 모듈의 최대 레벨 조회
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	int32 GetMaxLevel(UPRUpgradeModuleData* InModule) const;

	// 다음 레벨 비용 조회
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	float GetNextLevelCost(UPRUpgradeModuleData* InModule) const;

	/*~ Currency Operations ~*/

	// 화폐 조회
	UFUNCTION(BlueprintPure, Category = "Upgrade|Currency")
	float GetCurrency(FGameplayTag CurrencyTag) const;

	/*~ Purchase History ~*/

	// 구매한 모듈 목록 조회 (세이브용)
	UFUNCTION(BlueprintPure, Category = "Upgrade|History")
	const TArray<FPRUpgradePurchaseInfo>& GetPurchasedModules() const;

public:
	/*~ Delegates ~*/

	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradePurchased OnUpgradePurchased;

	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeCurrencyChanged OnCurrencyChanged;

private:
	// RogueliteSubsystem 바인딩
	void BindToRogueliteSubsystem();
	void UnbindFromRogueliteSubsystem();

	// RogueliteSubsystem 헬퍼
	URogueliteSubsystem* GetRogueliteSubsystem() const;

	// 이벤트 핸들러
	UFUNCTION()
	void HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue);

private:
	// 구매한 모듈 목록 (세이브/로드용)
	UPROPERTY()
	TArray<FPRUpgradePurchaseInfo> PurchasedModules;

	bool bIsBoundToSubsystem = false;
};
