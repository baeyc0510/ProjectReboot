// Fill out your copyright notice in the Description page of Project Settings.

#include "PRShopComponent.h"
#include "PRShopCatalog.h"
#include "RogueliteActionData.h"
#include "RoguelitePoolPreset.h"
#include "RogueliteSubsystem.h"

UPRShopComponent::UPRShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPRShopComponent::BeginPlay()
{
	Super::BeginPlay();

	GenerateShopItems();
}

void UPRShopComponent::GenerateShopItems()
{
	ShopItems.Empty();

	URogueliteSubsystem* RogueliteSS = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSS) || !IsValid(PoolPreset) || !IsValid(Catalog))
	{
		OnShopInventoryUpdated.Broadcast();
		return;
	}

	// PoolPreset 기반 쿼리로 ActionData 목록 획득
	TArray<URogueliteActionData*> QueriedActions = RogueliteSS->QuerySimple(PoolPreset, ShopItemCount);

	// 각 ActionData에 대응하는 가격 정보 검색
	for (URogueliteActionData* Action : QueriedActions)
	{
		if (!IsValid(Action))
		{
			continue;
		}

		const FPRShopItemInfo* FoundInfo = Catalog->FindInfoForAction(Action);
		if (!FoundInfo)
		{
			continue;
		}

		FPRShopItemEntry Entry;
		Entry.ItemInfo = *FoundInfo;
		Entry.bSoldOut = false;
		ShopItems.Add(Entry);
	}

	OnShopInventoryUpdated.Broadcast();
}

bool UPRShopComponent::TryPurchaseItem(int32 ItemIndex, FString& OutFailReason)
{
	if (!ShopItems.IsValidIndex(ItemIndex))
	{
		OutFailReason = TEXT("유효하지 않은 아이템 인덱스");
		return false;
	}

	FPRShopItemEntry& Entry = ShopItems[ItemIndex];

	if (Entry.bSoldOut)
	{
		OutFailReason = TEXT("이미 판매 완료된 상품");
		return false;
	}

	if (!IsValid(Entry.ItemInfo.ActionData))
	{
		OutFailReason = TEXT("유효하지 않은 상품 데이터");
		return false;
	}

	URogueliteSubsystem* RogueliteSS = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSS))
	{
		OutFailReason = TEXT("RogueliteSubsystem 없음");
		return false;
	}

	// 재화 확인
	const FGameplayTag CurrencyTag = Entry.ItemInfo.CurrencyTag;
	const float CurrentCurrency = RogueliteSS->GetStateValue(CurrencyTag);
	const float ItemPrice = Entry.ItemInfo.Price;

	if (CurrentCurrency < ItemPrice)
	{
		OutFailReason = TEXT("재화 부족");
		return false;
	}

	// 재화 차감
	RogueliteSS->AddStateValue(CurrencyTag, -ItemPrice);

	// 액션 획득
	URogueliteActionData* Action = Entry.ItemInfo.ActionData;
	if (IsValid(Action))
	{
		if (!RogueliteSS->AcquireAction(Action))
		{
			// 획득 실패 시 재화 롤백
			RogueliteSS->AddStateValue(CurrencyTag, ItemPrice);
			OutFailReason = TEXT("액션 획득 실패");
			return false;
		}
	}

	// 구매 완료 처리
	Entry.bSoldOut = true;

	OnShopItemPurchased.Broadcast(ItemIndex);
	OnShopInventoryUpdated.Broadcast();

	return true;
}

bool UPRShopComponent::CanPurchaseItem(int32 ItemIndex) const
{
	if (!ShopItems.IsValidIndex(ItemIndex))
	{
		return false;
	}

	const FPRShopItemEntry& Entry = ShopItems[ItemIndex];
	if (Entry.bSoldOut || !IsValid(Entry.ItemInfo.ActionData))
	{
		return false;
	}

	const float CurrentCurrency = GetCurrency(Entry.ItemInfo.CurrencyTag);
	return CurrentCurrency >= Entry.ItemInfo.Price;
}

float UPRShopComponent::GetCurrency(FGameplayTag CurrencyTag) const
{
	URogueliteSubsystem* RogueliteSS = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSS))
	{
		return 0.f;
	}

	return RogueliteSS->GetStateValue(CurrencyTag);
}
