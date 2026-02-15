// PRGA_Fire_Bullet.cpp
#include "PRGA_Fire_Bullet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
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

	const bool bIsScatter = Weapon->HasTag(TAG_Equipment_Weapon_Type_Scatter);

	if (bIsScatter)
	{
		// Scatter: 여러 펠릿 발사 (짧은 사거리)
		const float ScatterRange = 10000.f * ScatterRangeMultiplier;
		TArray<FVector> AllImpactPoints;

		// ScatterCount 어트리뷰트에서 펠릿 수 조회 (0 이하면 fallback 사용)
		int32 PelletCount = ScatterPelletCount;
		if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			const int32 AttrCount = FMath::TruncToInt(ASC->GetNumericAttribute(UPRWeaponAttributeSet::GetScatterCountAttribute()));
			if (AttrCount > 0)
			{
				PelletCount = AttrCount;
			}
		}

		FVector AimStart, AimDir;
		GetCameraAimInfo(AimStart, AimDir);

		for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
		{
			const TArray<FHitResult> HitResults = PerformHitscanWithSpread(ScatterSpreadAngle, ScatterRange);
			for (int32 HitIndex = 0; HitIndex < HitResults.Num(); ++HitIndex)
			{
				const FHitResult& HitResult = HitResults[HitIndex];
				Weapon->PlayImpact(HitResult);
				ApplyWeaponDamage(HitResult, HitIndex);
			}
			if (HitResults.Num() > 0)
			{
				AllImpactPoints.Add(HitResults.Last().ImpactPoint);
			}
			else
			{
				// 미히트 시 스프레드 방향의 최대 사거리 지점을 끝점으로 사용
				const float HalfAngleRad = FMath::DegreesToRadians(ScatterSpreadAngle * 0.5f);
				const FVector SpreadDir = FMath::VRandCone(AimDir, HalfAngleRad);
				AllImpactPoints.Add(AimStart + SpreadDir * ScatterRange);
			}
		}

		Weapon->PlayBulletTrail(AllImpactPoints);
	}
	else
	{
		// Single: 단일 히트스캔
		const TArray<FHitResult> HitResults = PerformHitscanWithSpread(BaseSpreadAngle);

		for (int32 HitIndex = 0; HitIndex < HitResults.Num(); ++HitIndex)
		{
			const FHitResult& HitResult = HitResults[HitIndex];
			Weapon->PlayImpact(HitResult);
			ApplyWeaponDamage(HitResult, HitIndex);
		}

		if (HitResults.Num() > 0)
		{
			Weapon->PlayBulletTrail({ HitResults.Last().ImpactPoint });
		}
		else
		{
			// 미히트 시 조준점을 끝점으로 사용
			Weapon->PlayBulletTrail({ GetAimPoint() });
		}
	}

	Weapon->OnFired();
}

UBulletWeaponInstance* UPRGA_Fire_Bullet::GetBulletWeapon() const
{
	return GetWeaponInstanceAs<UBulletWeaponInstance>();
}
