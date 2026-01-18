// PRGA_Fire_Bullet.cpp
#include "PRGA_Fire_Bullet.h"
#include "ProjectReboot/Equipment/Weapon/BulletWeaponInstance.h"

UPRGA_Fire_Bullet::UPRGA_Fire_Bullet()
{
	ActivationPolicy = EPRAbilityActivationPolicy::WhileInputHeld;
}

bool UPRGA_Fire_Bullet::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UBulletWeaponInstance* Weapon = GetBulletWeapon();
	if (!IsValid(Weapon))
	{
		return false;
	}

	return Weapon->CanFire();
}

void UPRGA_Fire_Bullet::OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::OnActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FireOnce();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPRGA_Fire_Bullet::FireOnce()
{
	UBulletWeaponInstance* Weapon = GetBulletWeapon();
	if (!IsValid(Weapon) || !Weapon->CanFire())
	{
		return;
	}

	FHitResult HitResult;
	bool bHit = PerformHitscanWithSpread(HitResult, BaseSpreadAngle);

	Weapon->PlayMuzzleFlash();

	if (bHit)
	{
		Weapon->PlayImpact(HitResult);
		ApplyWeaponDamage(HitResult);
	}

	Weapon->OnFired();
}

UBulletWeaponInstance* UPRGA_Fire_Bullet::GetBulletWeapon() const
{
	return GetWeaponInstanceAs<UBulletWeaponInstance>();
}
