#include "PRHUDViewModel.h"
#include "AbilitySystemComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"

UPRHUDViewModel::UPRHUDViewModel()
{
	// ViewModel Tag 설정
	ViewModelTag = TAG_UI_ViewModel_HUD;
}

// 무기 타입 태그 정의

void UPRHUDViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
	// HUD는 기본적으로 표시
	SetVisible(true);
}

void UPRHUDViewModel::Deinitialize()
{
	UnbindFromEquipmentManager();
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

	// 태그 변경 등록
	TagChangedDelegateHandle = InASC->RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::OnTagChanged);
	
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
		if (TagChangedDelegateHandle.IsValid())
		{
			BoundASC->RegisterGenericGameplayTagEvent().Remove(TagChangedDelegateHandle);
			TagChangedDelegateHandle.Reset();
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
		const bool bMaxChanged = !FMath::IsNearlyEqual(MaxHealth, NewMax);
		CurrentHealth = NewCurrent;
		MaxHealth = NewMax;
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

		if (bMaxChanged)
		{
			UpdateSegments(MaxHealth, OnHealthSegmentChanged);
		}
	}
}

void UPRHUDViewModel::SetShield(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentShield, NewCurrent) || !FMath::IsNearlyEqual(MaxShield, NewMax))
	{
		const bool bMaxChanged = !FMath::IsNearlyEqual(MaxShield, NewMax);
		CurrentShield = NewCurrent;
		MaxShield = NewMax;
		OnShieldChanged.Broadcast(CurrentShield, MaxShield);

		if (bMaxChanged)
		{
			UpdateSegments(MaxShield, OnShieldSegmentChanged);
		}
	}
}

void UPRHUDViewModel::SetStamina(float NewCurrent, float NewMax)
{
	if (!FMath::IsNearlyEqual(CurrentStamina, NewCurrent) || !FMath::IsNearlyEqual(MaxStamina, NewMax))
	{
		const bool bMaxChanged = !FMath::IsNearlyEqual(MaxStamina, NewMax);
		CurrentStamina = NewCurrent;
		MaxStamina = NewMax;
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UPRHUDViewModel::UpdateSegments(float MaxValue, FOnHUDSegmentChanged& SegmentDelegate)
{
	const int32 NumSegments = (UnitHealth > KINDA_SMALL_NUMBER) ? FMath::TruncToInt(MaxValue / UnitHealth) : 0;
	const float Spacing = NumSegments / 100.0f;
	SegmentDelegate.Broadcast(NumSegments, Spacing);
}

int32 UPRHUDViewModel::GetHealthNumSegments() const
{
	return (UnitHealth > KINDA_SMALL_NUMBER) ? FMath::TruncToInt(MaxHealth / UnitHealth) : 0;
}

float UPRHUDViewModel::GetHealthSpacing() const
{
	return GetHealthNumSegments() / 100.0f;
}

int32 UPRHUDViewModel::GetShieldNumSegments() const
{
	return (UnitHealth > KINDA_SMALL_NUMBER) ? FMath::TruncToInt(MaxShield / UnitHealth) : 0;
}

float UPRHUDViewModel::GetShieldSpacing() const
{
	return GetShieldNumSegments() / 100.0f;
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

void UPRHUDViewModel::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag.MatchesTag(TAG_Equipment_Weapon_Type) && !Tag.MatchesTagExact(TAG_Equipment_Weapon_Type))
	{
		HandleWeaponTagChanged(Tag, NewCount);
	}
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

	// 스태미나 어트리뷰트 바인딩
	BindAttributeDelegate(UPRCommonAttributeSet::GetStaminaAttribute(), &UPRHUDViewModel::HandleStaminaChanged);
	BindAttributeDelegate(UPRCommonAttributeSet::GetMaxStaminaAttribute(), &UPRHUDViewModel::HandleMaxStaminaChanged);

	// 초기 스태미나 값 설정
	bool bFoundStamina = false;
	bool bFoundMaxStamina = false;
	float ValStamina = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetStaminaAttribute(), bFoundStamina);
	float ValMaxStamina = BoundASC->GetGameplayAttributeValue(UPRCommonAttributeSet::GetMaxStaminaAttribute(), bFoundMaxStamina);
	SetStamina(ValStamina, ValMaxStamina);

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
			for (auto& OwnedTag : BoundASC->GetOwnedGameplayTags())
			{
				if (OwnedTag.MatchesTag(TAG_Equipment_Weapon_Type))
				{
					SetWeaponType(OwnedTag);
				}
			}
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

void UPRHUDViewModel::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	SetStamina(Data.NewValue, MaxStamina);
}

void UPRHUDViewModel::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	SetStamina(CurrentStamina, Data.NewValue);
}

void UPRHUDViewModel::BindToEquipmentManager(UPREquipmentManagerComponent* InEquipmentManager)
{
	UnbindFromEquipmentManager();

	if (!IsValid(InEquipmentManager))
	{
		return;
	}

	BoundEquipmentManager = InEquipmentManager;

	InEquipmentManager->OnEquipped.AddDynamic(this, &ThisClass::HandleEquipmentChanged);
	InEquipmentManager->OnUnequipped.AddDynamic(this, &ThisClass::HandleEquipmentChanged);

	// 초기 부품 아이콘 갱신
	UpdatePartIcons();
}

void UPRHUDViewModel::UnbindFromEquipmentManager()
{
	if (BoundEquipmentManager.IsValid())
	{
		BoundEquipmentManager->OnEquipped.RemoveDynamic(this, &ThisClass::HandleEquipmentChanged);
		BoundEquipmentManager->OnUnequipped.RemoveDynamic(this, &ThisClass::HandleEquipmentChanged);
	}

	BoundEquipmentManager.Reset();
	PartIcons.Reset();
}

void UPRHUDViewModel::HandleEquipmentChanged(FGameplayTag SlotTag, UEquipmentInstance* Instance, UPREquipActionData* ActionData)
{
	// 무기 슬롯 변경 시에만 부품 아이콘 갱신
	if (SlotTag.MatchesTag(TAG_Equipment_Slot_Weapon))
	{
		UpdatePartIcons();
	}
}

void UPRHUDViewModel::UpdatePartIcons()
{
	PartIcons.Reset();

	if (!BoundEquipmentManager.IsValid())
	{
		OnPartIconsChanged.Broadcast();
		return;
	}

	// 무기 부품 슬롯 순회
	const FGameplayTag PartSlots[] =
	{
		TAG_Equipment_Slot_Weapon_Barrel,
		TAG_Equipment_Slot_Weapon_Mag,
		TAG_Equipment_Slot_Weapon_Scope,
		TAG_Equipment_Slot_Weapon_Stock,
		TAG_Equipment_Slot_Weapon_Trigger,
	};

	for (const FGameplayTag& SlotTag : PartSlots)
	{
		UPREquipActionData* PartAction = BoundEquipmentManager->GetActionData(SlotTag);
		if (!IsValid(PartAction))
		{
			continue;
		}

		if (IsValid(PartAction->Icon))
		{
			PartIcons.Add(PartAction->Icon);
		}
	}

	OnPartIconsChanged.Broadcast();
}
