// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PRGameplayAbility.generated.h"

class UAnimMontage;

/**
 * 
 */

UENUM()
enum class EPRAbilityActivationPolicy
{
	OnInputTriggered,
	WhileInputHeld,
	OnGiven,
};

UCLASS()
class PROJECTREBOOT_API UPRGameplayAbility : public UGameplayAbility
	, public IPRPrewarmInterface
{
	GENERATED_BODY()
	
public:
	// 태그로 몽타주 찾기 공통 헬퍼
	UAnimMontage* FindMontageByGameplayTag(const FGameplayTag& MontageTag) const;

	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 에셋 목록 수집
	virtual void GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const override{};

	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override{};

	EPRAbilityActivationPolicy GetActivationPolicy() const {return ActivationPolicy;}
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EPRAbilityActivationPolicy ActivationPolicy;
};
