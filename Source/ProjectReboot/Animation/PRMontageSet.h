// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PRMontageSet.generated.h"

/**
 * 몽타주 변형 데이터
 */
USTRUCT(BlueprintType)
struct FPRMontageVariation
{
	GENERATED_BODY()

	// 몽타주 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> Montages;

	// 랜덤 몽타주 반환
	UAnimMontage* GetRandomMontage() const
	{
		if (Montages.IsEmpty())
		{
			return nullptr;
		}
		
		if (Montages.Num() == 1)
		{
			return Montages[0];
		}

		const int32 Index = FMath::RandRange(0, Montages.Num() - 1);
		return Montages[Index];
	}

	// 유효한 몽타주가 있는지 확인
	bool IsValid() const
	{
		return !Montages.IsEmpty();
	}
};

/**
 * Tag - Montage 매핑 데이터
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTREBOOT_API UPRMontageSet : public UDataAsset
{
	GENERATED_BODY()

public:
	// 태그별 몽타주 변형 매핑
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FPRMontageVariation> TagMontageMap;

public:
	// 태그에 해당하는 몽타주 랜덤 반환
	UFUNCTION(BlueprintPure)
	UAnimMontage* FindMontageByTag(const FGameplayTag& MontageTag) const;
};