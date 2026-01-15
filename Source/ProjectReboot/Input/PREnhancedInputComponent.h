// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "PRInputConfig.h"
#include "PREnhancedInputComponent.generated.h"


class UPRInputConfig;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPREnhancedInputComponent : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:
    UPREnhancedInputComponent(const FObjectInitializer& ObjectInitializer);

    template<class UserClass, typename FuncType>
    void BindInputActionWithTag( const UInputAction* InputAction, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,
        UserClass* Object, FuncType Func);
    
    template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindInputActionsByConfig(const UPRInputConfig* InputConfig, UserClass* Object, 
        PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc);
};

template <class UserClass, typename FuncType>
void UPREnhancedInputComponent::BindInputActionWithTag(const UInputAction* InputAction, const FGameplayTag& InputTag,
    ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
    if (!InputAction || !Object || !InputTag.IsValid())
    {
        return;
    }

    BindAction(InputAction, TriggerEvent, Object, Func, InputTag);
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UPREnhancedInputComponent::BindInputActionsByConfig(const UPRInputConfig* InputConfig, UserClass* Object,
    PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
{
    if (!InputConfig || !Object)
    {
        return;
    }

    for (const FInputActionMapping& Mapping : InputConfig->InputMappings)
    {
        BindInputActionWithTag(Mapping.InputAction, Mapping.InputTag, ETriggerEvent::Started, Object, PressedFunc);
        BindInputActionWithTag(Mapping.InputAction, Mapping.InputTag, ETriggerEvent::Completed, Object, ReleasedFunc);
    }
}
