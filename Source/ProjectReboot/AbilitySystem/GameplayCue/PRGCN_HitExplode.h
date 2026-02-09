// PRGCN_HitExplode.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "PRGCN_HitExplode.generated.h"

class UNiagaraSystem;
class USoundBase;
class UWeaponInstance;

/**
 * 피격 리액션시 폭발 이펙트 GameplayCue
 * - HitResult의 ImpactPoint/ImpactNormal에서 Niagara 및 사운드 재생
 * - 무기 인스턴스의 ExplodeVFX/ExplodeSound 우선 사용
 * - 무기 설정 없으면 DefaultExplodeVFX/DefaultExplodeSound 사용
 */
UCLASS()
class PROJECTREBOOT_API UPRGCN_HitExplode : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UPRGCN_HitExplode();

	/*~ UGameplayCueNotify_Static Interface ~*/
	virtual bool OnExecute_Implementation(AActor* MyTarget,
		const FGameplayCueParameters& Parameters) const override;

protected:
	// 시전자의 무기 인스턴스 획득
	UWeaponInstance* GetWeaponInstanceFromContext(const FGameplayCueParameters& Parameters) const;

	// 피격 이펙트 스폰
	void SpawnExplodeVFX(UWorld* World, const FVector& Location, const FVector& Normal,
		UNiagaraSystem* OverrideVFX) const;

	// 피격 사운드 재생
	void PlayExplodeSound(UWorld* World, const FVector& Location, USoundBase* OverrideSound) const;
protected:
	// 피격 이펙트 (Niagara)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|VFX")
	TObjectPtr<UNiagaraSystem> DefaultExplodeVFX;

	// 이펙트 방향 반전 (-X 방향 에셋 보정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|VFX")
	bool bFlipExplodeNormal = false;
	
	// 이펙트 스케일
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|VFX")
	FVector ExplodeScale = FVector::OneVector;
	
	// 피격 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Sound")
	TObjectPtr<USoundBase> DefaultExplodeSound;

	// 사운드 볼륨 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Sound")
	float SoundVolumeMultiplier = 1.0f;
};
