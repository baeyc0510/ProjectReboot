// PRGA_Fire_Beam.cpp
#include "PRGA_Fire_Beam.h"
#include "ProjectReboot/Equipment/Weapon/BeamWeaponInstance.h"

UPRGA_Fire_Beam::UPRGA_Fire_Beam()
{
	ActivationPolicy = EPRAbilityActivationPolicy::WhileInputHeld;
}

bool UPRGA_Fire_Beam::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UBeamWeaponInstance* Weapon = GetBeamWeapon();
	if (!IsValid(Weapon))
	{
		return false;
	}

	return Weapon->CanFire();
}

void UPRGA_Fire_Beam::OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::OnActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UBeamWeaponInstance* Weapon = GetBeamWeapon();
	if (!IsValid(Weapon))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Weapon->StartFiring();
	LastDamageTime = 0.0f;

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().SetTimer(
			BeamTickTimerHandle,
			FTimerDelegate::CreateUObject(this, &UPRGA_Fire_Beam::TickBeam, 0.016f),
			0.016f,
			true
		);
	}
}

void UPRGA_Fire_Beam::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(BeamTickTimerHandle);
	}

	UBeamWeaponInstance* Weapon = GetBeamWeapon();
	if (IsValid(Weapon))
	{
		Weapon->StopFiring();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGA_Fire_Beam::TickBeam(float DeltaTime)
{
	UBeamWeaponInstance* Weapon = GetBeamWeapon();
	if (!IsValid(Weapon))
	{
		K2_EndAbility();
		return;
	}

	Weapon->ConsumeEnergy(DeltaTime);

	if (!Weapon->IsFiring())
	{
		K2_EndAbility();
		return;
	}

	LastDamageTime += DeltaTime;
	if (LastDamageTime >= DamageInterval)
	{
		LastDamageTime = 0.0f;

		FHitResult HitResult;
		if (PerformHitscan(HitResult))
		{
			ApplyWeaponDamage(HitResult);
			Weapon->PlayImpact(HitResult);
		}
	}
}

UBeamWeaponInstance* UPRGA_Fire_Beam::GetBeamWeapon() const
{
	return GetWeaponInstanceAs<UBeamWeaponInstance>();
}
