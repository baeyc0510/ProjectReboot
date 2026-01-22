// BeamWeaponInstance.cpp
#include "BeamWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

bool UBeamWeaponInstance::CanFire() const
{
	if (GetCurrentEnergy() <= 0.0f)
	{
		return false;
	}

	return true;
}

void UBeamWeaponInstance::StartFiring()
{
	if (bIsFiring)
	{
		return;
	}

	if (!CanFire())
	{
		return;
	}

	bIsFiring = true;
	PlayMuzzleFlash();
}

void UBeamWeaponInstance::StopFiring()
{
	if (!bIsFiring)
	{
		return;
	}

	bIsFiring = false;
}

float UBeamWeaponInstance::GetCurrentEnergy() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (IsValid(ASC))
	{
		bool bFound = false;
		float Energy = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFound);
		if (bFound)
		{
			return Energy;
		}
	}

	return 0.0f;
}

void UBeamWeaponInstance::ConsumeEnergy(float DeltaTime)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!IsValid(ASC))
	{
		return;
	}

	bool bFoundEnergy = false;
	bool bFoundFireRate = false;

	float CurrentEnergy = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFoundEnergy);
	float FireRate = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetFireRateAttribute(), bFoundFireRate);
	
	if (!bFoundEnergy || !bFoundFireRate)
	{
		return;
	}

	float DrainRate =  FireRate / 60;
	float EnergyToConsume = DrainRate * DeltaTime;
	float NewEnergy = FMath::Max(0.0f, CurrentEnergy - EnergyToConsume);

	ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetAmmoAttribute(), NewEnergy);

	// 에너지 소진 시 자동 정지
	if (NewEnergy <= 0.0f)
	{
		StopFiring();
	}
}
