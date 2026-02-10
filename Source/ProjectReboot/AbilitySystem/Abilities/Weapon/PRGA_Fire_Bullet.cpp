// PRGA_Fire_Bullet.cpp
#include "PRGA_Fire_Bullet.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Equipment/Weapon/BulletWeaponInstance.h"

UPRGA_Fire_Bullet::UPRGA_Fire_Bullet()
{
	ActivationPolicy = EPRAbilityActivationPolicy::WhileInputHeld;
	ActivationRequiredTags.AddTag(TAG_State_Aiming);
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

	if (!IsValid(OwnerCharacter))
	{
		OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	
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
	
	// 몽타주 재생
	if (IsValid(OwnerCharacter) && IsValid(BulletFireMontage))
	{
		OwnerCharacter->PlayAnimMontage(BulletFireMontage);
	}
	
	// Muzzle VFX 재생
	Weapon->PlayMuzzleFlash();
	
	// 스프레드 적용 히트스캔
	const TArray<FHitResult> HitResults = PerformHitscanWithSpread(BaseSpreadAngle);

	if (HitResults.Num() > 0)
	{
		// 관통 순서대로 데미지 적용
		for (int32 HitIndex = 0; HitIndex < HitResults.Num(); ++HitIndex)
		{
			const FHitResult& HitResult = HitResults[HitIndex];
			Weapon->PlayImpact(HitResult);
			ApplyWeaponDamage(HitResult, HitIndex);
		}
	}

	Weapon->OnFired();
}

UBulletWeaponInstance* UPRGA_Fire_Bullet::GetBulletWeapon() const
{
	return GetWeaponInstanceAs<UBulletWeaponInstance>();
}
