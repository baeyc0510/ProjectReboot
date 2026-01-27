// MissileWeaponInstance.h
#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "MissileWeaponInstance.generated.h"

class USkeletalMeshComponent;

// 미사일 탄환 메시 설정 (Equipment Tag 조합별)
USTRUCT(BlueprintType)
struct FMissileAmmoMeshConfig
{
	GENERATED_BODY()

	// 이 설정이 적용될 Equipment Tag 조합 (모두 포함되어야 매칭)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Missile")
	FGameplayTagContainer RequiredTags;

	// 탄환 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Missile")
	TObjectPtr<USkeletalMesh> AmmoMesh;

	// 탄환 메시를 부착할 소켓 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Missile")
	TArray<FName> AmmoMeshSocketNames;
};

/**
 * 미사일 기반 무기 인스턴스
 * Projectile 스폰 방식, 장전된 미사일 관리, 다중 총구 지원
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UMissileWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

public:
	/*~ UWeaponInstance Interface ~*/
	virtual bool CanFire() const override;
	virtual void OnFired() override;
	virtual bool CanReload() const override;
	virtual bool IsReloading() const override;
	virtual void StartReload() override;
	virtual void FinishReload() override;
	virtual void CancelReload() override;
	virtual FTransform GetMuzzleTransform() const override;

	/*~ UEquipmentInstance Interface ~*/
	virtual void DestroyAllVisuals() override;

	/*~ UEquipmentInstance Interface ~*/
	virtual void OnEquipmentTagsChanged() override;

	// 현재 장전된 미사일 수 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile")
	int32 GetLoadedMissiles() const;

	// 미사일 소모
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile")
	void ConsumeMissile();

	// 폭발 반경 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile")
	float GetExplosionRadius() const;

	// 미사일 메시 가시성 업데이트
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile")
	void UpdateMissileVisuals();

	/*~ 락온 타겟 관리 ~*/

	// 락온 타겟 설정 (TargetLockComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile|LockOn")
	void SetLockedTargets(const TArray<AActor*>& Targets);

	// 락온 타겟 목록 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile|LockOn")
	TArray<AActor*> GetLockedTargets() const;

	// 락온 타겟 하나 소비 (발사 시 호출, 가장 먼저 락온된 타겟 반환 후 목록에서 제거)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile|LockOn")
	AActor* ConsumeLockedTarget();

	// 락온 타겟 모두 제거
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile|LockOn")
	void ClearLockedTargets();

	// 현재 락온된 타겟 수
	UFUNCTION(BlueprintCallable, Category = "Weapon|Missile|LockOn")
	int32 GetLockedTargetCount() const;

protected:
	// 탄환 메시 컴포넌트 스폰
	void SpawnAmmoMeshComponents();

	// 탄환 메시 컴포넌트 제거
	void DestroyAmmoMeshComponents();

protected:
	// Equipment Tag별 탄환 메시 설정 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Missile")
	TArray<FMissileAmmoMeshConfig> AmmoMeshConfigs;

	// 재장전 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Missile")
	bool bIsReloading = false;

	// 생성된 탄환 메시 컴포넌트 목록
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Missile")
	TArray<TObjectPtr<USkeletalMeshComponent>> AmmoMeshComponents;

	// 스폰할 Projectile 클래스 (GA에서 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Missile")
	TSubclassOf<AActor> ProjectileClass;

	// 현재 발사 인덱스 (순차 발사용)
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Missile")
	int32 CurrentFireIndex = 0;

	// 락온된 타겟 목록 (TWeakObjectPtr는 BP 미지원, GetLockedTargets() 사용)
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> LockedTargets;
};
