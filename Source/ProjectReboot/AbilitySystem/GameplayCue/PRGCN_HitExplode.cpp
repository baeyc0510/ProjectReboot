// PRGCN_HitExplode.cpp
#include "PRGCN_HitExplode.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/WeaponInstance.h"

UPRGCN_HitExplode::UPRGCN_HitExplode()
{
	// 태그 자동 매칭: GameplayCue.Weapon.Impact
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Impact"));
}

bool UPRGCN_HitExplode::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!IsValid(MyTarget))
	{
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	// 무기 인스턴스에서 VFX/Sound 획득
	UNiagaraSystem* ExplodeVFX = nullptr;
	USoundBase* ExplodeSound = nullptr;

	if (UWeaponInstance* WeaponInstance = GetWeaponInstanceFromContext(Parameters))
	{
		const FWeaponFXSettings& FXSettings = WeaponInstance->GetVFXSettings();
		ExplodeVFX = FXSettings.ExplodeVFX;
		ExplodeSound = FXSettings.ExplodeSound;
	}

	// EffectContext에서 HitResult 추출
	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	if (HitResult)
	{
		SpawnExplodeVFX(World, HitResult->ImpactPoint, HitResult->ImpactNormal, ExplodeVFX);
		PlayExplodeSound(World, HitResult->ImpactPoint, ExplodeSound);
		return true;
	}

	// Fallback: Parameters의 Location 사용
	if (!Parameters.Location.IsZero())
	{
		FVector Normal = Parameters.Normal.IsZero() ? FVector::UpVector : Parameters.Normal;
		SpawnExplodeVFX(World, Parameters.Location, Normal, ExplodeVFX);
		PlayExplodeSound(World, Parameters.Location, ExplodeSound);
		return true;
	}

	return false;
}

UWeaponInstance* UPRGCN_HitExplode::GetWeaponInstanceFromContext(const FGameplayCueParameters& Parameters) const
{
	// Instigator 획득
	AActor* Instigator = Parameters.EffectContext.GetInstigator();
	if (!IsValid(Instigator))
	{
		return nullptr;
	}

	// EquipmentManager 획득
	UPREquipmentManagerComponent* EquipmentManager = UPREquipmentBlueprintLibrary::GetEquipmentManager(Instigator);
	if (!IsValid(EquipmentManager))
	{
		return nullptr;
	}

	// AggregatedSourceTags에서 무기 슬롯 태그 검색
	const FGameplayTagContainer& SourceTags = Parameters.AggregatedSourceTags;

	// TAG_Equipment_Slot_Weapon 하위 태그 검색
	for (const FGameplayTag& Tag : SourceTags)
	{
		if (Tag.MatchesTag(TAG_Equipment_Slot_Weapon))
		{
			// 해당 슬롯의 장비 인스턴스 획득
			UEquipmentInstance* Equipment = EquipmentManager->GetEquipmentInstance(Tag);
			if (UWeaponInstance* Weapon = Cast<UWeaponInstance>(Equipment))
			{
				return Weapon;
			}
		}
	}

	return nullptr;
}

void UPRGCN_HitExplode::SpawnExplodeVFX(UWorld* World, const FVector& Location, const FVector& Normal,
	UNiagaraSystem* OverrideVFX) const
{
	// Override VFX 우선, 없으면 Default 사용
	UNiagaraSystem* VFXToSpawn = IsValid(OverrideVFX) ? OverrideVFX : DefaultExplodeVFX.Get();
	if (!IsValid(VFXToSpawn))
	{
		return;
	}

	const FVector ImpactNormal = bFlipExplodeNormal ? -Normal : Normal;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		VFXToSpawn,
		Location,
		ImpactNormal.Rotation(),
		ExplodeScale,
		true,  // bAutoDestroy
		true,  // bAutoActivate
		ENCPoolMethod::AutoRelease
	);
}

void UPRGCN_HitExplode::PlayExplodeSound(UWorld* World, const FVector& Location, USoundBase* OverrideSound) const
{
	// Override Sound 우선, 없으면 Default 사용
	USoundBase* SoundToPlay = IsValid(OverrideSound) ? OverrideSound : DefaultExplodeSound.Get();
	if (!IsValid(SoundToPlay))
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		World,
		SoundToPlay,
		Location,
		SoundVolumeMultiplier
	);
}
