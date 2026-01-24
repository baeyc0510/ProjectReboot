// WeaponTypes.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WeaponTypes.generated.h"

/*~ 전방 선언 ~*/
class UNiagaraSystem;
class USoundBase;

// 무기 VFX 설정
USTRUCT(BlueprintType)
struct FWeaponVFXSettings
{
	GENERATED_BODY()

	// 총구 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

	// 궤적 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TrailVFX;

	// 기본 피격 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> DefaultImpactVFX;

	// 피격 이펙트 방향 반전 여부 (-X 방향 에셋 보정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	bool bFlipImpactNormal = false;

	// 발사 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> FireSound;

	// 피격 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> ImpactSound;
};

// 총구 슬롯 설정 (Equipment Tag 조합별)
USTRUCT(BlueprintType)
struct FWeaponMuzzleSlotConfig
{
	GENERATED_BODY()

	// 이 설정이 적용될 Equipment Tag 조합 (모두 포함되어야 매칭)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Muzzle")
	FGameplayTagContainer RequiredTags;

	// 사용할 총구 소켓 목록 (1개 이상)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Muzzle")
	TArray<FName> MuzzleSocketNames;
};
