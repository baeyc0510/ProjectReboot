// PRGA_Fire_Missile.cpp
#include "PRGA_Fire_Missile.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/Projectile/PRMissileProjectile.h"
#include "ProjectReboot/Equipment/Weapon/MissileWeaponInstance.h"

UPRGA_Fire_Missile::UPRGA_Fire_Missile()
{
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
	
	ActivationRequiredTags.AddTag(TAG_State_Aiming);
}

void UPRGA_Fire_Missile::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	Super::GetPrewarmChildren(OutChildren);
	if (IsValid(DefaultProjectileClass))
	{
		OutChildren.Add(DefaultProjectileClass);
	}
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

	if (!IsValid(OwnerCharacter))
	{
		OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	
	FireAllMissiles();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPRGA_Fire_Missile::FireAllMissiles()
{
	UMissileWeaponInstance* Weapon = GetMissileWeapon();
	if (!IsValid(Weapon) || !Weapon->CanFire())
	{
		return;
	}
	
	if (IsValid(OwnerCharacter) && IsValid(MissileFireMontage))
	{
		OwnerCharacter->PlayAnimMontage(MissileFireMontage);
	}
	
	if (Weapon->GetLockedTargetCount() > 0)
	{
		// 락온된 모든 타겟에 대해 잔탄이 있는 동안 발사
		while (Weapon->CanFire() && Weapon->GetLockedTargetCount() > 0)
		{
			AActor* HomingTarget = Weapon->ConsumeLockedTarget();
			FireSingleMissile(Weapon, HomingTarget);
		}
	}
	else
	{
		FireSingleMissile(Weapon, nullptr);
	}
}

void UPRGA_Fire_Missile::FireSingleMissile(UMissileWeaponInstance* Weapon, AActor* HomingTarget)
{
	if (!IsValid(Weapon) || !Weapon->CanFire())
	{
		return;
	}

	FTransform MuzzleTransform = Weapon->GetMuzzleTransform();

	// 총구에서 조준점을 향하는 방향 계산
	FVector AimPoint = GetAimPoint();
	FVector LaunchDir = (AimPoint - MuzzleTransform.GetLocation()).GetSafeNormal();
	MuzzleTransform.SetRotation(LaunchDir.ToOrientationQuat());

	// 발사체 스폰
	APRMissileProjectile* Projectile = SpawnProjectile(MuzzleTransform);
	if (IsValid(Projectile))
	{
		// 발사체 초기화 (데미지, 폭발 반경, 유도 타겟 등)
		InitializeProjectile(Projectile, HomingTarget);

		// 발사 속도 및 유도 파라미터 설정
		Projectile->LaunchInDirection(LaunchDir, ProjectileSpeed);
		Projectile->SetNavigationConstant(NavigationConstant);
		Projectile->SetMaxNavigationAcceleration(MaxNavigationAcceleration);
		Projectile->SetMaxRange(MaxRange);
	}

	Weapon->PlayMuzzleFlash();
	Weapon->OnFired();
}

APRMissileProjectile* UPRGA_Fire_Missile::SpawnProjectile(const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !DefaultProjectileClass)
	{
		return nullptr;
	}

	// 발사체 소유자 지정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(SpawnParams.Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return World->SpawnActor<APRMissileProjectile>(DefaultProjectileClass, SpawnTransform, SpawnParams);
}

void UPRGA_Fire_Missile::InitializeProjectile(APRMissileProjectile* Projectile, AActor* HomingTarget)
{
	if (!IsValid(Projectile))
	{
		return;
	}

	UMissileWeaponInstance* Weapon = GetMissileWeapon();

	// 폭발 반경 설정
	if (IsValid(Weapon))
	{
		Projectile->SetExplosionRadius(Weapon->GetExplosionRadius());
	}

	// 데미지 GE 설정
	if (IsValid(DamageEffectClass))
	{
		Projectile->SetDamageEffectClass(DamageEffectClass);
	}

	// Instigator ASC 설정
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		Projectile->SetInstigatorASC(ASC);
	}

	// 무기 슬롯 태그 설정 (GCN에서 무기 인스턴스 조회용)
	if (WeaponSlotTag.IsValid())
	{
		Projectile->SetWeaponSlotTag(WeaponSlotTag);
	}

	// 유도 타겟 설정
	if (IsValid(HomingTarget))
	{
		Projectile->SetHomingTarget(HomingTarget);
	}
}

UMissileWeaponInstance* UPRGA_Fire_Missile::GetMissileWeapon() const
{
	return GetWeaponInstanceAs<UMissileWeaponInstance>();
}
