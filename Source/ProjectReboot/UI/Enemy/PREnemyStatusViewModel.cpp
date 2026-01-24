// PREnemyStatusViewModel.cpp
#include "PREnemyStatusViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

void UPREnemyStatusViewModel::InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForActor(InTargetActor, InLocalPlayer);

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InTargetActor))
	{
		BindToASC(ASC);
	}
	
	if (APREnemyCharacter* EnemyCharacter = Cast<APREnemyCharacter>(InTargetActor))
	{
		SetEnemyDisplayName(EnemyCharacter->GetEnemyDisplayName());
	}
}

void UPREnemyStatusViewModel::Deinitialize()
{
	UnbindFromASC();
	Super::Deinitialize();
}

void UPREnemyStatusViewModel::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();

	if (!InASC)
	{
		return;
	}

	BoundASC = InASC;
	UpdateAttributeBindings();
}

void UPREnemyStatusViewModel::UnbindFromASC()
{
	ClearAttributeBindings();
	BoundASC.Reset();
}

void UPREnemyStatusViewModel::SetEnemyDisplayName(const FText& InDisplayName)
{
	DisplayName = InDisplayName;
	OnEnemyDisplayNameChanged.Broadcast(DisplayName);
}

void UPREnemyStatusViewModel::SetHealth(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentHealth, NewCurrent) || !FMath::IsNearlyEqual(MaxHealth, NewMax))
	{
		CurrentHealth = NewCurrent;
		MaxHealth = NewMax;
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UPREnemyStatusViewModel::SetShield(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentShield, NewCurrent) || !FMath::IsNearlyEqual(MaxShield, NewMax))
	{
		CurrentShield = NewCurrent;
		MaxShield = NewMax;
		OnShieldChanged.Broadcast(CurrentShield, MaxShield);
	}
}

void UPREnemyStatusViewModel::UpdateAttributeBindings()
{
	ClearAttributeBindings();

	if (!BoundASC.IsValid())
	{
		return;
	}

	// 체력 바인딩
	HealthChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetHealthAttribute()
	).AddUObject(this, &UPREnemyStatusViewModel::HandleHealthChanged);

	MaxHealthChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetMaxHealthAttribute()
	).AddUObject(this, &UPREnemyStatusViewModel::HandleMaxHealthChanged);

	// 실드 바인딩
	ShieldChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetShieldAttribute()
	).AddUObject(this, &UPREnemyStatusViewModel::HandleShieldChanged);

	MaxShieldChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetMaxShieldAttribute()
	).AddUObject(this, &UPREnemyStatusViewModel::HandleMaxShieldChanged);

	// 초기 값 설정
	bool bFoundHealth = false;
	bool bFoundMaxHealth = false;
	const float ValHealth = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetHealthAttribute(), bFoundHealth);
	const float ValMaxHealth = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);
	SetHealth(ValHealth, ValMaxHealth);

	bool bFoundShield = false;
	bool bFoundMaxShield = false;
	const float ValShield = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetShieldAttribute(), bFoundShield);
	const float ValMaxShield = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxShieldAttribute(), bFoundMaxShield);
	SetShield(ValShield, ValMaxShield);
}

void UPREnemyStatusViewModel::ClearAttributeBindings()
{
	if (BoundASC.IsValid())
	{
		auto UnbindSafe = [&](const FGameplayAttribute& Attribute, FDelegateHandle& Handle)
		{
			if (Handle.IsValid() && BoundASC.IsValid())
			{
				BoundASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
				Handle.Reset();
			}
		};

		UnbindSafe(UPRCommonAttributeSet::GetHealthAttribute(), HealthChangeHandle);
		UnbindSafe(UPRCommonAttributeSet::GetMaxHealthAttribute(), MaxHealthChangeHandle);
		UnbindSafe(UPRCommonAttributeSet::GetShieldAttribute(), ShieldChangeHandle);
		UnbindSafe(UPRCommonAttributeSet::GetMaxShieldAttribute(), MaxShieldChangeHandle);
	}

	HealthChangeHandle.Reset();
	MaxHealthChangeHandle.Reset();
	ShieldChangeHandle.Reset();
	MaxShieldChangeHandle.Reset();
}

void UPREnemyStatusViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(Data.NewValue, MaxHealth);
}

void UPREnemyStatusViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(CurrentHealth, Data.NewValue);
}

void UPREnemyStatusViewModel::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	SetShield(Data.NewValue, MaxShield);
}

void UPREnemyStatusViewModel::HandleMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	SetShield(CurrentShield, Data.NewValue);
}
