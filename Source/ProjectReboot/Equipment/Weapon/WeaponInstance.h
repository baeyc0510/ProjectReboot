// WeaponInstance.h
#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "ProjectReboot/Equipment/EquipmentInstance.h"
#include "WeaponInstance.generated.h"

class UAbilitySystemComponent;

/**
 * 무기 인스턴스 기본 클래스
 * 발사 로직은 GA_Fire에서 처리하고, 이 클래스는 상태와 VFX 데이터를 관리한다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UWeaponInstance : public UEquipmentInstance
{
	GENERATED_BODY()

public:
	// 발사 가능 여부
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool CanFire() const;

	// 발사 후 호출
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void OnFired();

	// 재장전 가능 여부
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool CanReload() const;

	// 재장전 중 여부
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool IsReloading() const;

	// 재장전 시작
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartReload();

	// 재장전 완료
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void FinishReload();

	// 재장전 취소
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void CancelReload();

	// 총구 위치/방향 반환 (기본: 현재 슬롯 설정의 첫 번째 소켓)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual FTransform GetMuzzleTransform() const;

	// FX 재생
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	virtual void PlayMuzzleFlash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	void PlayImpact(const FHitResult& HitResult);

	// 총알 궤적 VFX 재생
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	void PlayBulletTrail(const TArray<FVector>& ImpactPoints);

	// VFX 설정 Getter
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	const FWeaponFXSettings& GetVFXSettings() const { return FXSettings; }

	// 현재 활성화된 총구 정보 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	const TArray<FActiveMuzzleInfo>& GetActiveMuzzles() const { return ActiveMuzzles; }

protected:
	/*~ UEquipmentInstance Interface ~*/
	virtual void OnEquipmentTagsChanged() override;
	virtual void OnEquipped() override;

	// Equipment Tag에 따른 총구 소켓 설정 업데이트
	void UpdateMuzzleSlotConfig();

	// VFX Pre-warm (셰이더 컴파일 히칭 방지)
	void WarmupVFX();

protected:
	// VFX 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	FWeaponFXSettings FXSettings;

	// Equipment Tag별 총구 슬롯 설정 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponMuzzleSlotConfig> MuzzleSlotConfigs;

	// 기본 총구 소켓 이름 (매칭되는 설정이 없을 경우 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName DefaultMuzzleSocketName = TEXT("Muzzle");

	// 현재 활성화된 총구 정보 목록
	UPROPERTY()
	TArray<FActiveMuzzleInfo> ActiveMuzzles;

	// ASC 참조 헬퍼
	UAbilitySystemComponent* GetOwnerASC() const;
};
