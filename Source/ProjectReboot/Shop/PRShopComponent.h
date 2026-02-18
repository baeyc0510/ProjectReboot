#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PRShopTypes.h"
#include "PRShopComponent.generated.h"

class URogueliteActionData;
class URoguelitePoolPreset;
class UPRShopCatalog;

/**
 * 상점 컴포넌트
 * 상점 아이템 생성, 구매 로직 담당
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPRShopComponent();

	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;

public:
	/*~ UPRShopComponent Interface ~*/

	// 상점 아이템 생성 (PoolPreset 쿼리 기반)
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void GenerateShopItems();

	// 아이템 구매 시도
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool TryPurchaseItem(int32 ItemIndex, FString& OutFailReason);

	// 아이템 구매 가능 여부
	UFUNCTION(BlueprintPure, Category = "Shop")
	bool CanPurchaseItem(int32 ItemIndex) const;

	// 지정 화폐 보유량 반환
	UFUNCTION(BlueprintPure, Category = "Shop")
	float GetCurrency(FGameplayTag CurrencyTag) const;

	// 현재 상점 아이템 목록 반환
	UFUNCTION(BlueprintPure, Category = "Shop")
	const TArray<FPRShopItemEntry>& GetShopItems() const { return ShopItems; }

public:
	/*~ Delegates ~*/

	// 아이템 구매 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopItemPurchased OnShopItemPurchased;

	// 상점 인벤토리 갱신 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopInventoryUpdated OnShopInventoryUpdated;

protected:
	/*~ Settings ~*/

	// 상점 아이템 쿼리용 풀 프리셋
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TObjectPtr<URoguelitePoolPreset> PoolPreset;

	// 상점 카탈로그
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TObjectPtr<UPRShopCatalog> Catalog;

	// 실제 진열할 상품 수
	UPROPERTY(EditDefaultsOnly, Category = "Shop", meta = (ClampMin = "1"))
	int32 ShopItemCount = 6;

private:
	// 현재 진열된 상품 목록
	UPROPERTY()
	TArray<FPRShopItemEntry> ShopItems;
};
