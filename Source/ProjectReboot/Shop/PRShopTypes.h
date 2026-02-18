// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "PRShopTypes.generated.h"

class URogueliteActionData;
class UTexture2D;

/**
 * 상점 아이템 정보 구조체
 * ActionData + 고정 가격
 */
USTRUCT(BlueprintType)
struct FPRShopItemInfo
{
	GENERATED_BODY()

	// 실제 적용할 액션 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<URogueliteActionData> ActionData = nullptr;

	// 사용할 화폐 종류 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CurrencyTag = TAG_Currency_Core;

	// 고정 가격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"))
	float Price = 100.f;

	// 표시 이름 반환
	FText GetDisplayName() const;

	// 설명 반환
	FText GetDescription() const;

	// 아이콘 반환
	UTexture2D* GetIcon() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopItemPurchased, int32, ItemIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopInventoryUpdated);

/**
 * 상점 아이템 엔트리 (런타임)
 */
USTRUCT(BlueprintType)
struct FPRShopItemEntry
{
	GENERATED_BODY()

	// 상품 정보
	UPROPERTY(BlueprintReadOnly)
	FPRShopItemInfo ItemInfo;

	// 구매 완료 여부
	UPROPERTY(BlueprintReadOnly)
	bool bSoldOut = false;
};
