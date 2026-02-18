// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PRShopTypes.h"
#include "PRShopCatalog.generated.h"

class URogueliteActionData;

/**
 * 상점 카탈로그
 * 모든 상점 아이템의 가격 정보를 보유
 */
UCLASS()
class PROJECTREBOOT_API UPRShopCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	// 상품 가격 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop")
	TArray<FPRShopItemInfo> Items;

	// ActionData에 대응하는 가격 정보 검색
	const FPRShopItemInfo* FindInfoForAction(const URogueliteActionData* InAction) const;
};
