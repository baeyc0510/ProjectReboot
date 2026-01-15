// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "PRGameplayAbility.h"
#include "PRAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UPRAbilitySystemComponent();

	void AbilityInputPressed(const FGameplayTag& InputTag);
	void AbilityInputReleased(const FGameplayTag& InputTag);

	// APlayerController::PostProcessInput에서 호출 - 입력 일괄 처리
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

private:
	EPRAbilityActivationPolicy GetActivationPolicy(const FGameplayAbilitySpec& Spec) const;
	
private:
	// 이번 입력처리에 눌린 Ability 핸들
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// 이번 입력처리에 해제된 Ability 핸들
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 현재 입력처리에 눌려 있는 Ability 핸들
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
};
