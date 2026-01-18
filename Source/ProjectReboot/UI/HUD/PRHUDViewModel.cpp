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
	BulletTagHandle = InASC->RegisterGameplayTagEvent(
		TAG_Equipment_Weapon_Type_Bullet,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UPRHUDViewModel::HandleWeaponTagChanged);

	BeamTagHandle = InASC->RegisterGameplayTagEvent(
		TAG_Equipment_Weapon_Type_Beam,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UPRHUDViewModel::HandleWeaponTagChanged);

	MissileTagHandle = InASC->RegisterGameplayTagEvent(
		TAG_Equipment_Weapon_Type_Missile,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UPRHUDViewModel::HandleWeaponTagChanged);

	// 체력 어트리뷰트 바인딩
	HealthChangeHandle = InASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetHealthAttribute()
	).AddUObject(this, &UPRHUDViewModel::HandleHealthChanged);

	MaxHealthChangeHandle = InASC->GetGameplayAttributeValueChangeDelegate(
		UPRCommonAttributeSet::GetMaxHealthAttribute()
	).AddUObject(this, &UPRHUDViewModel::HandleMaxHealthChanged);

	// 초기 체력 값 설정
	bool bFoundHealth = false;
	bool bFoundMaxHealth = false;
	float ValHealth = InASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetHealthAttribute(), bFoundHealth);
	float ValMaxHealth = InASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);
	SetHealth(ValHealth, ValMaxHealth);

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
	ClearAttributeBindings();

	if (BoundASC.IsValid())
	{
		if (BulletTagHandle.IsValid())
		{
			BoundASC->RegisterGameplayTagEvent(TAG_Equipment_Weapon_Type_Bullet, EGameplayTagEventType::NewOrRemoved).Remove(BulletTagHandle);
			BulletTagHandle.Reset();
		}
		if (BeamTagHandle.IsValid())
		{
			BoundASC->RegisterGameplayTagEvent(TAG_Equipment_Weapon_Type_Beam, EGameplayTagEventType::NewOrRemoved).Remove(BeamTagHandle);
			BeamTagHandle.Reset();
		}
		if (MissileTagHandle.IsValid())
		{
			BoundASC->RegisterGameplayTagEvent(TAG_Equipment_Weapon_Type_Missile, EGameplayTagEventType::NewOrRemoved).Remove(MissileTagHandle);
			MissileTagHandle.Reset();
		}

		if (HealthChangeHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(UPRCommonAttributeSet::GetHealthAttribute()).Remove(HealthChangeHandle);
			HealthChangeHandle.Reset();
		}
		if (MaxHealthChangeHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(UPRCommonAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangeHandle);
			MaxHealthChangeHandle.Reset();
		}
	}

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

void UPRHUDViewModel::SetHealth(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentHealth, NewCurrent) || !FMath::IsNearlyEqual(MaxHealth, NewMax))
	{
		CurrentHealth = NewCurrent;
		MaxHealth = NewMax;
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UPRHUDViewModel::SetWeaponType(const FGameplayTag& NewType)
{
	if (WeaponTypeTag != NewType)
	{
		WeaponTypeTag = NewType;
		OnWeaponTypeChanged.Broadcast(WeaponTypeTag);
		UpdateAttributesBindings();
	}
}

void UPRHUDViewModel::ClearAttributeBindings()
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

		// 모든 가능한 어트리뷰트 바인딩 해제
		UnbindSafe(UPRWeaponAttributeSet::GetAmmoAttribute(), AmmoChangeHandle);
		UnbindSafe(UPRWeaponAttributeSet::GetMaxAmmoAttribute(), MaxAmmoChangeHandle);
		
		UnbindSafe(UPRWeaponAttributeSet::GetEnergyAttribute(), AmmoChangeHandle);
		UnbindSafe(UPRWeaponAttributeSet::GetMaxEnergyAttribute(), MaxAmmoChangeHandle);
		
		UnbindSafe(UPRWeaponAttributeSet::GetLoadedMissilesAttribute(), AmmoChangeHandle);
		UnbindSafe(UPRWeaponAttributeSet::GetMaxLoadedMissilesAttribute(), MaxAmmoChangeHandle);
		
		UnbindSafe(UPRCommonAttributeSet::GetHealthAttribute(), HealthChangeHandle);
		UnbindSafe(UPRCommonAttributeSet::GetMaxHealthAttribute(), MaxHealthChangeHandle);
	}
	
	AmmoChangeHandle.Reset();
	MaxAmmoChangeHandle.Reset();
}

void UPRHUDViewModel::UpdateAttributesBindings()
{
	ClearAttributeBindings();

	if (!BoundASC.IsValid())
	{
		return;
	}

	if (WeaponTypeTag.MatchesTag(TAG_Equipment_Weapon_Type_Bullet))
	{
		AmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetAmmoAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleAmmoChanged);

		MaxAmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetMaxAmmoAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleMaxAmmoChanged);

		// 초기 업데이트
		bool bFoundCurrent = false;
		bool bFoundMax = false;
		float ValCurrent = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFoundCurrent);
		float ValMax = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxAmmoAttribute(), bFoundMax);
		SetAmmo(FMath::TruncToInt(ValCurrent), FMath::TruncToInt(ValMax));
	}
	else if (WeaponTypeTag.MatchesTag(TAG_Equipment_Weapon_Type_Beam))
	{
		AmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetEnergyAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleEnergyChanged);

		MaxAmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetMaxEnergyAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleMaxEnergyChanged);

		// 초기 업데이트
		bool bFoundCurrent = false;
		bool bFoundMax = false;
		float ValCurrent = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetEnergyAttribute(), bFoundCurrent);
		float ValMax = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxEnergyAttribute(), bFoundMax);
		SetAmmo(FMath::TruncToInt(ValCurrent), FMath::TruncToInt(ValMax));
	}
	else if (WeaponTypeTag.MatchesTag(TAG_Equipment_Weapon_Type_Missile))
	{
		AmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetLoadedMissilesAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleMissileChanged);

		MaxAmmoChangeHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
			UPRWeaponAttributeSet::GetMaxLoadedMissilesAttribute()
		).AddUObject(this, &UPRHUDViewModel::HandleMaxMissileChanged);

		// 초기 업데이트
		bool bFoundCurrent = false;
		bool bFoundMax = false;
		float ValCurrent = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetLoadedMissilesAttribute(), bFoundCurrent);
		float ValMax = BoundASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxLoadedMissilesAttribute(), bFoundMax);
		SetAmmo(FMath::TruncToInt(ValCurrent), FMath::TruncToInt(ValMax));
	}
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

void UPRHUDViewModel::HandleEnergyChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(FMath::TruncToInt(Data.NewValue), MaxAmmo);
}

void UPRHUDViewModel::HandleMaxEnergyChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(CurrentAmmo, FMath::TruncToInt(Data.NewValue));
}

void UPRHUDViewModel::HandleMissileChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(FMath::TruncToInt(Data.NewValue), MaxAmmo);
}

void UPRHUDViewModel::HandleMaxMissileChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(CurrentAmmo, FMath::TruncToInt(Data.NewValue));
}

void UPRHUDViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(Data.NewValue, MaxHealth);
}

void UPRHUDViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	SetHealth(CurrentHealth, Data.NewValue);
}
