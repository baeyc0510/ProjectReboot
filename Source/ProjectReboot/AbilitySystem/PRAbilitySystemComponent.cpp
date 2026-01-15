// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAbilitySystemComponent.h"

#include "PRGameplayAbility.h"

UPRAbilitySystemComponent::UPRAbilitySystemComponent()
{
}

void UPRAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
    Super::OnGiveAbility(AbilitySpec);

    if (GetActivationPolicy(AbilitySpec) == EPRAbilityActivationPolicy::OnGiven)
    {
        TryActivateAbility(AbilitySpec.Handle);
    }
}

void UPRAbilitySystemComponent::AbilityInputPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
        {
            InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
            InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
        }
    }
}

void UPRAbilitySystemComponent::AbilityInputReleased(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
    {
        if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
        {
            InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
            InputHeldSpecHandles.Remove(AbilitySpec.Handle);
        }
    }
}

void UPRAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
    static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
    AbilitiesToActivate.Reset();

    // WhileInputHeld: 입력 유지 중인 Ability 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
    {
        if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (Spec->Ability && !Spec->IsActive())
            {
                if (GetActivationPolicy(*Spec) == EPRAbilityActivationPolicy::WhileInputHeld)
                {
                    AbilitiesToActivate.AddUnique(SpecHandle);
                }
            }
        }
    }

    // OnInputTriggered: 이번 프레임에 눌린 Ability 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
    {
        if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (Spec->Ability)
            {
                Spec->InputPressed = true;

                if (Spec->IsActive())
                {
                    AbilitySpecInputPressed(*Spec);
                }
                else if (GetActivationPolicy(*Spec) == EPRAbilityActivationPolicy::OnInputTriggered)
                {
                    AbilitiesToActivate.AddUnique(SpecHandle);
                }
            }
        }
    }

    // 일괄 활성화
    for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToActivate)
    {
        TryActivateAbility(SpecHandle);
    }

    // Released 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
    {
        if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (Spec->Ability)
            {
                Spec->InputPressed = false;

                if (Spec->IsActive())
                {
                    AbilitySpecInputReleased(*Spec);
                }
            }
        }
    }

    // 프레임 입력 초기화
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
}

void UPRAbilitySystemComponent::ClearAbilityInput()
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();
}

EPRAbilityActivationPolicy UPRAbilitySystemComponent::GetActivationPolicy(const FGameplayAbilitySpec& Spec) const
{
    if (const UPRGameplayAbility* PRAbility = Cast<UPRGameplayAbility>(Spec.Ability))
    {
        return PRAbility->GetActivationPolicy();
    }
    return EPRAbilityActivationPolicy::None;
}