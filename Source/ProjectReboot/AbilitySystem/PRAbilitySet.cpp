// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAbilitySet.h"

#include "AbilitySystemComponent.h"

void FPRAbilitySetHandles::RemoveFromAbilitySystem()
{
	if (!CachedASC.IsValid())
	{
		Reset();
		return;
	}

	UAbilitySystemComponent* ASC = CachedASC.Get();

	for (const FGameplayAbilitySpecHandle& Handle : AbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	Reset();
}
