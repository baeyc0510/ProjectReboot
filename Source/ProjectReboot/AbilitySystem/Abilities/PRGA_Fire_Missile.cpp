// PRGA_Fire_Missile.cpp
#include "PRGA_Fire_Missile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectReboot/Equipment/Weapon/MissileWeaponInstance.h"

UPRGA_Fire_Missile::UPRGA_Fire_Missile()
{
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
}

bool UPRGA_Fire_Missile::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UMissileWeaponInstance* Weapon = GetMissileWeapon();
	if (!IsValid(Weapon))
	{
		return false;
	}

	return Weapon->CanFire();
}

void UPRGA_Fire_Missile::OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::OnActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FireMissile();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPRGA_Fire_Missile::FireMissile()
{
	UMissileWeaponInstance* Weapon = GetMissileWeapon();
	if (!IsValid(Weapon) || !Weapon->CanFire())
	{
		return;
	}

	FTransform MuzzleTransform = Weapon->GetMuzzleTransform();

	// 총구에서 조준점을 향하는 방향 계산
	FVector AimPoint = GetAimPoint();
	FVector LaunchDir = (AimPoint - MuzzleTransform.GetLocation()).GetSafeNormal();
	MuzzleTransform.SetRotation(LaunchDir.ToOrientationQuat());

	AActor* Projectile = SpawnProjectile(MuzzleTransform);
	if (IsValid(Projectile))
	{
		UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>();
		if (IsValid(MoveComp))
		{
			MoveComp->InitialSpeed = ProjectileSpeed;
			MoveComp->MaxSpeed = ProjectileSpeed;
			MoveComp->Velocity = LaunchDir * ProjectileSpeed;
		}
	}

	Weapon->PlayMuzzleFlash();
	Weapon->OnFired();
}

AActor* UPRGA_Fire_Missile::SpawnProjectile(const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(DefaultProjectileClass))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(SpawnParams.Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return World->SpawnActor<AActor>(DefaultProjectileClass, SpawnTransform, SpawnParams);
}

UMissileWeaponInstance* UPRGA_Fire_Missile::GetMissileWeapon() const
{
	return GetWeaponInstanceAs<UMissileWeaponInstance>();
}
