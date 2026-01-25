// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PRCommonAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 캐릭터 공통 (플레이어, 적) 어트리뷰트 세트
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCommonAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/*~ UAttributeSet Interfaces ~*/
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

protected:
	/*~ UPRCommonAttributeSet Interfaces ~*/
	
	// Health 변경 처리 (사망 체크 포함)
	void HandleHealthChanged(const FGameplayEffectModCallbackData& Data);

	// 사망 처리 (CombatInterface::Die 호출)
	void HandleDeath(const FGameplayEffectModCallbackData& Data);
	
private:
	void ReportDamageEventIfNeeded(const FGameplayEffectModCallbackData& Data);

	void UpdateCharacterWalkSpeed();

public:
	/*~ Movement ~*/
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, MoveSpeed)

	/*~ Health ~*/
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, MaxHealth)

	/*~ Shield ~*/
	// 실드
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, Shield)

	// 최대 실드
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, MaxShield)

	/*~ Hit Stagger ~*/
	// 경직 수치
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Stagger;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, Stagger)

	// 피격 임계치
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData HitImmunity;
	ATTRIBUTE_ACCESSORS(UPRCommonAttributeSet, HitImmunity)
};