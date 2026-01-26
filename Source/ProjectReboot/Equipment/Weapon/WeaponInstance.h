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
	// 발사 가능 여부 (쿨다운 체크)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool CanFire() const;

	// 발사 후 호출 (마지막 발사 시간 기록)
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

	// VFX 재생
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	void PlayMuzzleFlash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	void PlayImpact(const FHitResult& HitResult);

	// VFX 설정 Getter
	UFUNCTION(BlueprintCallable, Category = "Weapon|VFX")
	const FWeaponVFXSettings& GetVFXSettings() const { return VFXSettings; }

	// 현재 활성화된 총구 소켓 목록 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	const TArray<FName>& GetActiveMuzzleSockets() const { return ActiveMuzzleSockets; }

protected:
	/*~ UEquipmentInstance Interface ~*/
	virtual void OnEquipmentTagsChanged() override;

	// Equipment Tag에 따른 총구 소켓 설정 업데이트
	void UpdateMuzzleSlotConfig();

protected:
	// VFX 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	FWeaponVFXSettings VFXSettings;

	// Equipment Tag별 총구 슬롯 설정 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponMuzzleSlotConfig> MuzzleSlotConfigs;

	// 기본 총구 소켓 이름 (매칭되는 설정이 없을 경우 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName DefaultMuzzleSocketName = TEXT("Muzzle");

	// 현재 활성화된 총구 소켓 목록
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TArray<FName> ActiveMuzzleSockets;

	// 마지막 발사 시간
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float LastFireTime = 0.0f;

	// 발사 간격 (초) - AttributeSet의 FireRate로부터 계산
	float GetFireInterval() const;

	// ASC 참조 헬퍼
	UAbilitySystemComponent* GetOwnerASC() const;
};
