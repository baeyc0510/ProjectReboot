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
		// 고정 간격으로 빔 데미지 처리
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

		// 관통 히트 처리
		const TArray<FHitResult> HitResults = PerformHitscan();
		if (HitResults.Num() > 0)
		{
			// 관통 순서대로 데미지 적용
			for (int32 HitIndex = 0; HitIndex < HitResults.Num(); ++HitIndex)
			{
				const FHitResult& HitResult = HitResults[HitIndex];
				ApplyWeaponDamage(HitResult, HitIndex);
				Weapon->PlayImpact(HitResult);
			}
		}
	}
}

UBeamWeaponInstance* UPRGA_Fire_Beam::GetBeamWeapon() const
{
	return GetWeaponInstanceAs<UBeamWeaponInstance>();
}
