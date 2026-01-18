// BulletWeaponInstance.h
#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "BulletWeaponInstance.generated.h"

/**
 * 탄환 기반 무기 인스턴스 (라이플 등)
 * Hitscan 방식, 탄창/재장전 시스템
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UBulletWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

public:
	/*~ UWeaponInstance Interface ~*/
	virtual bool CanFire() const override;
	virtual void OnFired() override;

	// 재장전 중 여부
	UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
	bool IsReloading() const { return bIsReloading; }

	// 재장전 시작
	UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
	void StartReload();

	// 재장전 완료
	UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
	void FinishReload();

	// 현재 탄약 조회 (AttributeSet에서)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
	int32 GetCurrentAmmo() const;

	// 탄약 소모
	UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
	void ConsumeAmmo(int32 Amount = 1);

protected:
	// 재장전 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet")
	bool bIsReloading = false;
};
