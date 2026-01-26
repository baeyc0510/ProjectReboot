#include "PRHUDViewModel.h"
#include "AbilitySystemComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

// 무기 타입 태그 정의

void UPRHUDViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
}

void UPRHUDViewModel::Deinitialize()
{
	UnbindFromASC();
	Super::Deinitialize();
}

void UPRHUDViewModel::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();

	if (!InASC)
	{
		return;
	}

	BoundASC = InASC;

	// 무기 타입 태그 변경 등록
	WeaponTypeDelegateHandle = InASC->RegisterGameplayTagEvent(
		TAG_Equipment_Weapon_Type,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UPRHUDViewModel::HandleWeaponTagChanged);

	// 어트리뷰트 바인딩
	UpdateAttributesBindings();
	
	// 현재 무기 상태 확인
	if (InASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Bullet))
	{
		SetWeaponType(TAG_Equipment_Weapon_Type_Bullet);
	}
	else if (InASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Beam))
	{
		SetWeaponType(TAG_Equipment_Weapon_Type_Beam);
	}
	else if (InASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Missile))
	{
		SetWeaponType(TAG_Equipment_Weapon_Type_Missile);
	}
	else
	{
		SetWeaponType(FGameplayTag::EmptyTag);
	}
}

void UPRHUDViewModel::UnbindFromASC()
{
	// 태그 이벤트 바인딩 제거
	if (BoundASC.IsValid())
	{
		if (WeaponTypeDelegateHandle.IsValid())
		{
			BoundASC->UnregisterGameplayTagEvent(WeaponTypeDelegateHandle,TAG_Equipment_Weapon_Type,EGameplayTagEventType::NewOrRemoved);
			WeaponTypeDelegateHandle.Reset();
		}
	}

	// 어트리뷰트 바인딩 제거
	ClearAttributeBindings();
	
	BoundASC.Reset();
}

void UPRHUDViewModel::SetAmmo(int32 NewCurrent, int32 NewMax)
{
	if (CurrentAmmo != NewCurrent || MaxAmmo != NewMax)
	{
		CurrentAmmo = NewCurrent;
		MaxAmmo = NewMax;
		OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	}
}

void UPRHUDViewModel::SetReserveAmmo(int32 NewCurrent, int32 NewMax)
{
	if (CurrentReserveAmmo != NewCurrent || MaxReserveAmmo != NewMax)
	{
		CurrentReserveAmmo = NewCurrent;
		MaxReserveAmmo = NewMax;
		OnReserveAmmoChanged.Broadcast(CurrentReserveAmmo, MaxReserveAmmo);
	}
}

void UPRHUDViewModel::SetHealth(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentHealth, NewCurrent) || !FMath::IsNearlyEqual(MaxHealth, NewMax))
	{
		CurrentHealth = NewCurrent;
		MaxHealth = NewMax;
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UPRHUDViewModel::SetShield(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentShield, NewCurrent) || !FMath::IsNearlyEqual(MaxShield, NewMax))
	{
		CurrentShield = NewCurrent;
		MaxShield = NewMax;
		OnShieldChanged.Broadcast(CurrentShield, MaxShield);
	}
}

void UPRHUDViewModel::SetWeaponType(const FGameplayTag& NewType)
{
	if (WeaponTypeTag != NewType)
	{
		WeaponTypeTag = NewType;
		OnWeaponTypeChanged.Broadcast(WeaponTypeTag);
	}
}

void UPRHUDViewModel::ClearAttributeBindings()
{
	if (BoundASC.IsValid())
	{
		for (auto& Binding : AttributeBindings)
		{
			if (!BoundASC.IsValid() || !Binding.IsValid())
			{
				continue;
			}
			BoundASC->GetGameplayAttributeValueChangeDelegate(Binding.Attribute).Remove(Binding.DelegateHandle);
			Binding.DelegateHandle.Reset();
		}
	}
	
	AttributeBindings.Reset();
}

void UPRHUDViewModel::BindAttributeDelegate(const FGameplayAttribute& Attribute, void (UPRHUDViewModel::*Handler)(const FOnAttributeChangeData&))
{
	if (!BoundASC.IsValid())
	{
		return;
	}

	const FDelegateHandle DelegateHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, Handler);

	FHUDAttributeBinding NewBinding;
	NewBinding.Attribute = Attribute;
	NewBinding.DelegateHandle = DelegateHandle;
	AttributeBindings.Add(NewBinding);
}

void UPRHUDViewModel::UpdateAttributesBindings()
{
	ClearAttributeBindings();

	if (!BoundASC.IsValid())
	{
		return;
	}
	
	// 체력 어트리뷰트 바인딩
	BindAttributeDelegate(UPRCommonAttributeSet::GetHealthAttribute(), &UPRHUDViewModel::HandleHealthChanged);
	BindAttributeDelegate(UPRCommonAttributeSet::GetMaxHealthAttribute(), &UPRHUDViewModel::HandleMaxHealthChanged);

	// 초기 체력 값 설정
	bool bFoundHealth = false;
	bool bFoundMaxHealth = false;
	float ValHealth = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetHealthAttribute(), bFoundHealth);
	float ValMaxHealth = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);
	SetHealth(ValHealth, ValMaxHealth);

	// 실드 어트리뷰트 바인딩
	BindAttributeDelegate(UPRCommonAttributeSet::GetShieldAttribute(), &UPRHUDViewModel::HandleShieldChanged);
	BindAttributeDelegate(UPRCommonAttributeSet::GetMaxShieldAttribute(), &UPRHUDViewModel::HandleMaxShieldChanged);

	// 초기 실드 값 설정
	bool bFoundShield = false;
	bool bFoundMaxShield = false;
	float ValShield = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetShieldAttribute(), bFoundShield);
	float ValMaxShield = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxShieldAttribute(), bFoundMaxShield);
	SetShield(ValShield, ValMaxShield);
	
	// Ammo 어트리뷰트 바인딩
	BindAttributeDelegate(UPRWeaponAttributeSet::GetAmmoAttribute(), &UPRHUDViewModel::HandleAmmoChanged);
	BindAttributeDelegate(UPRWeaponAttributeSet::GetMaxAmmoAttribute(), &UPRHUDViewModel::HandleMaxAmmoChanged);
	BindAttributeDelegate(UPRWeaponAttributeSet::GetReserveAmmoAttribute(), &UPRHUDViewModel::HandleReserveAmmoChanged);
	BindAttributeDelegate(UPRWeaponAttributeSet::GetMaxReserveAmmoAttribute(), &UPRHUDViewModel::HandleMaxReserveAmmoChanged);

	// 초기 업데이트
	bool bFoundCurrent = false;
	bool bFoundMax = false;
	float ValCurrent = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFoundCurrent);
	float ValMax = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxAmmoAttribute(), bFoundMax);
	SetAmmo(FMath::TruncToInt(ValCurrent), FMath::TruncToInt(ValMax));

	bool bFoundReserve = false;
	bool bFoundMaxReserve = false;
	float ValReserve = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetReserveAmmoAttribute(), bFoundReserve);
	float ValMaxReserve = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxReserveAmmoAttribute(), bFoundMaxReserve);
	SetReserveAmmo(FMath::TruncToInt(ValReserve), FMath::TruncToInt(ValMaxReserve));
}

void UPRHUDViewModel::HandleWeaponTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 무기 태그 추가 시 해당 타입으로 전환, 제거 시 다른 태그 확인
	if (NewCount > 0)
	{
		SetWeaponType(Tag);
	}
	else if (Tag == WeaponTypeTag)
	{
		if (BoundASC.IsValid())
		{
			if (BoundASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Bullet)) SetWeaponType(TAG_Equipment_Weapon_Type_Bullet);
			else if (BoundASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Beam)) SetWeaponType(TAG_Equipment_Weapon_Type_Beam);
			else if (BoundASC->HasMatchingGameplayTag(TAG_Equipment_Weapon_Type_Missile)) SetWeaponType(TAG_Equipment_Weapon_Type_Missile);
			else SetWeaponType(FGameplayTag::EmptyTag);
		}
	}
}

void UPRHUDViewModel::HandleAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(FMath::TruncToInt(Data.NewValue), MaxAmmo);
}

void UPRHUDViewModel::HandleMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(CurrentAmmo, FMath::TruncToInt(Data.NewValue));
}

void UPRHUDViewModel::HandleReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetReserveAmmo(FMath::TruncToInt(Data.NewValue), MaxReserveAmmo);
}

void UPRHUDViewModel::HandleMaxReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetReserveAmmo(CurrentReserveAmmo, FMath::TruncToInt(Data.NewValue));
}

void UPRHUDViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(Data.NewValue, MaxHealth);
}

void UPRHUDViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(CurrentHealth, Data.NewValue);
}

void UPRHUDViewModel::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	SetShield(Data.NewValue, MaxShield);
}

void UPRHUDViewModel::HandleMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	SetShield(CurrentShield, Data.NewValue);
}
