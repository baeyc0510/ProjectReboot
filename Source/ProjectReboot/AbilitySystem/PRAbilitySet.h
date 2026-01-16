// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "PRAbilitySet.generated.h"


struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;
class UGameplayEffect;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FPRAbilityEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	FGameplayTagContainer DynamicTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	int32 Level = 1;
	
	bool IsValid() const
	{
		return AbilityClass != nullptr;
	}
};

USTRUCT(BlueprintType)
struct FPREffectEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	FGameplayTagContainer DynamicTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Ability")
	float Level = 1.f;
	
	bool IsValid() const
	{
		return EffectClass != nullptr;
	}
};


/**
 * 부여된 Ability/Effect 핸들을 저장하는 구조체
 */
USTRUCT(BlueprintType)
struct FPRAbilitySetHandles
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> EffectHandles;

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	
	void Reset()
	{
		AbilityHandles.Reset();
		EffectHandles.Reset();
	}

	bool IsValid() const
	{
		return AbilityHandles.Num() > 0 || EffectHandles.Num() > 0;
	}
	
	void RemoveFromAbilitySystem();
};

UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRAbilitySet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 적용할 Ability 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FPRAbilityEntry> Abilities;
	
	// 적용할 Effect 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FPREffectEntry> Effects;
};