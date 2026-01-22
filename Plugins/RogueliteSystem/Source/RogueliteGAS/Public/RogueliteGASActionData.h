#pragma once

#include "CoreMinimal.h"
#include "RogueliteActionData.h"
#include "RogueliteGASTypes.h"
#include "RogueliteGASActionData.generated.h"

class UGameplayAbility;
class UGameplayEffect;


USTRUCT(BlueprintType)
struct FRogueliteAbilityEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTagContainer DynamicTags;
	
	bool IsValidData() const;
};

USTRUCT(BlueprintType)
struct FRogueliteEffectEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTagContainer DynamicTags;
	
	bool IsValidData() const;
};

/**
 * GAS 확장 ActionData
 * Ability/Effect를 정의하고 트리거 조건을 설정
 */
UCLASS()
class ROGUELITEGAS_API URogueliteGASActionData : public URogueliteActionData
{
	GENERATED_BODY()

public:
	/*~ GAS 요소 ~*/

	// 부여할 Ability 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FRogueliteAbilityEntry> Abilities;

	// 적용할 Effect 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FRogueliteEffectEntry> Effects;
	
	// TagsToGrant의 태그들을 AbilitySystemComponent에 적용할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	bool bGrantTagsToAbilitySystem;
	
	/*~ 트리거 조건 ~*/

	// 트리거 조건 (TriggerEventTag가 비어있으면 패시브)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Trigger")
	FRogueliteTriggerCondition TriggerCondition;

	/*~ 스케일링 ~*/

	// 스택 변경 시 Effect 처리 방식
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Scaling")
	ERogueliteStackScalingMode StackScalingMode = ERogueliteStackScalingMode::SetByCaller;

	// 스택 수를 전달받을 SetByCaller 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Scaling")
	FGameplayTag StacksSetByCallerTag;

public:
	// 패시브 여부 확인
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	bool HasTrigger() const { return TriggerCondition.HasTrigger(); }

	// 트리거 이벤트 태그 반환
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	FGameplayTag GetTriggerEventTag() const { return TriggerCondition.TriggerEventTag; }

	// GAS 요소 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	bool HasGASElements() const;
};