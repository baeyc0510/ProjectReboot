// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PRUpgradeManagerComponent.generated.h"

class URogueliteSubsystem;
class UPRUpgradeModuleData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradePurchased, UPRUpgradeModuleData*, Module, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpgradeCurrencyChanged, FGameplayTag, CurrencyTag, float, OldValue, float, NewValue);

/**
 * 업그레이드 상태 관리 및 구매 로직을 담당하는 컴포넌트
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPRUpgradeManagerComponent();

protected:
	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
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

public:
	/*~ Delegates ~*/

	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradePurchased OnUpgradePurchased;

	UPROPERTY(BlueprintAssignable, Category = "Upgrade|Events")
	FOnUpgradeCurrencyChanged OnCurrencyChanged;

private:
	// Subsystem 바인딩
	void BindToRogueliteSubsystem();
	void UnbindFromRogueliteSubsystem();

	// Subsystem 헬퍼
	URogueliteSubsystem* GetRogueliteSubsystem() const;

	// 이벤트 핸들러
	UFUNCTION()
	void HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue);

private:
	bool bIsBoundToSubsystem = false;
};
