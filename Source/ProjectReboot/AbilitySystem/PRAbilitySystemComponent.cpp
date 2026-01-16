// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAbilitySystemComponent.h"

#include "PRAbilitySet.h"
#include "PRGameplayAbility.h"

UPRAbilitySystemComponent::UPRAbilitySystemComponent()
{
}

void UPRAbilitySystemComponent::GiveAbilityEntry(const FPRAbilityEntry& AbilityEntry,
    FGameplayAbilitySpecHandle& OutHandle)
{
    if (!AbilityEntry.IsValid())
    {
        return;
    }
    
    FGameplayAbilitySpec AbilitySpec(AbilityEntry.AbilityClass, AbilityEntry.Level);
    AbilitySpec.GetDynamicSpecSourceTags().AppendTags(AbilityEntry.DynamicTags);

    OutHandle = GiveAbility(AbilitySpec);
}

void UPRAbilitySystemComponent::ApplyEffectEntry(const FPREffectEntry& EffectEntry, FActiveGameplayEffectHandle& OutHandle)
{
    if (!EffectEntry.IsValid())
    {
        return;
    }
    
    FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
    ContextHandle.AddSourceObject(GetOwner());

    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectEntry.EffectClass, EffectEntry.Level, ContextHandle);
    
    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->AppendDynamicAssetTags(EffectEntry.DynamicTags);
        OutHandle = ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UPRAbilitySystemComponent::GiveAbilitySet(const UPRAbilitySet* AbilitySet, FPRAbilitySetHandles& OutHandles)
{
    OutHandles.Reset();
    
    if (!AbilitySet)
    {
        return;
    }

    OutHandles.AbilityHandles.Reserve(AbilitySet->Abilities.Num());
    OutHandles.EffectHandles.Reserve(AbilitySet->Effects.Num());

    for (const FPRAbilityEntry& Entry : AbilitySet->Abilities)
    {
        FGameplayAbilitySpecHandle Handle;
        GiveAbilityEntry(Entry, Handle);
        
        if (Handle.IsValid())
        {
            OutHandles.AbilityHandles.Add(Handle);
        }
    }

    for (const FPREffectEntry& Entry : AbilitySet->Effects)
    {
        FActiveGameplayEffectHandle Handle;
        ApplyEffectEntry(Entry, Handle);
        
        if (Handle.IsValid())
        {
            OutHandles.EffectHandles.Add(Handle);
        }
    }
}

void UPRAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
    Super::OnGiveAbility(AbilitySpec);

    if (GetActivationPolicy(AbilitySpec) == EPRAbilityActivationPolicy::OnGiven)
    {
        TryActivateAbility(AbilitySpec.Handle);
    }
}

void UPRAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputPressed(Spec);
    
    if (!Spec.IsActive())
    {
        return;
    }
    
    // WaitInputPressed Task에 이벤트 전달하기위해 InvokeRelicatedEvent 사용
    UGameplayAbility* AbilityCDO = Spec.Ability;
    EGameplayAbilityInstancingPolicy::Type InstancingPolicy = AbilityCDO->GetInstancingPolicy();
        
    if (InstancingPolicy == EGameplayAbilityInstancingPolicy::Type::InstancedPerActor)
    {
        if (UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance())
        {
            FGameplayAbilityActivationInfo ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());    
        }
    }
    else
    {
        for (UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances())
        {
            FGameplayAbilityActivationInfo ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
        }
    }
}

void UPRAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputReleased(Spec);
    
    if (!Spec.IsActive())
    {
        return;
    }
    
    // WaitInputReleased Task에 이벤트 전달하기위해 InvokeRelicatedEvent 사용
    UGameplayAbility* AbilityCDO = Spec.Ability;
    EGameplayAbilityInstancingPolicy::Type InstancingPolicy = AbilityCDO->GetInstancingPolicy();
        
    if (InstancingPolicy == EGameplayAbilityInstancingPolicy::Type::InstancedPerActor)
    {
        if (UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance())
        {
            FGameplayAbilityActivationInfo ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, ActivationInfo.GetActivationPredictionKey());    
        }
    }
    else
    {
        for (UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances())
        {
            FGameplayAbilityActivationInfo ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
        }
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
    return EPRAbilityActivationPolicy::OnInputTriggered;
}