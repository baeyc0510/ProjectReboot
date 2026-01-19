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
		float Energy = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetEnergyAttribute(), bFound);
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
	bool bFoundDrainRate = false;

	float CurrentEnergy = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetEnergyAttribute(), bFoundEnergy);
	float DrainRate = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetEnergyDrainRateAttribute(), bFoundDrainRate);

	if (!bFoundEnergy || !bFoundDrainRate)
	{
		return;
	}

	float EnergyToConsume = DrainRate * DeltaTime;
	float NewEnergy = FMath::Max(0.0f, CurrentEnergy - EnergyToConsume);

	ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetEnergyAttribute(), NewEnergy);

	// 에너지 소진 시 자동 정지
	if (NewEnergy <= 0.0f)
	{
		StopFiring();
	}
}
