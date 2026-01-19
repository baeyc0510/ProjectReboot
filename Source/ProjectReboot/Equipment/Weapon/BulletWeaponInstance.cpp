// BulletWeaponInstance.cpp
#include "BulletWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

bool UBulletWeaponInstance::CanFire() const
{
	if (bIsReloading)
	{
		return false;
	}

	if (GetCurrentAmmo() <= 0)
	{
		return false;
	}

	return Super::CanFire();
}

void UBulletWeaponInstance::OnFired()
{
	Super::OnFired();
	ConsumeAmmo(1);
}

void UBulletWeaponInstance::StartReload()
{
	if (bIsReloading)
	{
		return;
	}

	bIsReloading = true;
}

void UBulletWeaponInstance::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}

	bIsReloading = false;

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!IsValid(ASC))
	{
		return;
	}

	// AttributeSet에서 값 조회
	bool bFoundAmmo = false;
	bool bFoundMaxAmmo = false;
	bool bFoundReserve = false;

	float CurrentAmmo = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFoundAmmo);
	float MaxAmmo = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetMaxAmmoAttribute(), bFoundMaxAmmo);
	float ReserveAmmo = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetReserveAmmoAttribute(), bFoundReserve);

	if (!bFoundAmmo || !bFoundMaxAmmo || !bFoundReserve)
	{
		return;
	}

	// 필요한 탄약량 계산
	float AmmoNeeded = MaxAmmo - CurrentAmmo;
	float AmmoToTransfer = FMath::Min(AmmoNeeded, ReserveAmmo);

	if (AmmoToTransfer > 0.0f)
	{
		// 탄창 충전
		ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetAmmoAttribute(), CurrentAmmo + AmmoToTransfer);
		// 예비 탄약 감소
		ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetReserveAmmoAttribute(), ReserveAmmo - AmmoToTransfer);
	}
}

int32 UBulletWeaponInstance::GetCurrentAmmo() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (IsValid(ASC))
	{
		bool bFound = false;
		float Ammo = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFound);
		if (bFound)
		{
			return FMath::FloorToInt(Ammo);
		}
	}

	return 0;
}

void UBulletWeaponInstance::ConsumeAmmo(int32 Amount)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!IsValid(ASC))
	{
		return;
	}

	bool bFound = false;
	float CurrentAmmo = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFound);
	if (bFound)
	{
		float NewAmmo = FMath::Max(0.0f, CurrentAmmo - Amount);
		ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetAmmoAttribute(), NewAmmo);
	}
}
