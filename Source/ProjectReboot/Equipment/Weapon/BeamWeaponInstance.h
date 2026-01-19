// BeamWeaponInstance.h
#pragma once

#include "CoreMinimal.h"
#include "WeaponInstance.h"
#include "BeamWeaponInstance.generated.h"

/**
 * 빔 기반 무기 인스턴스 (레이저 등)
 * 지속형 Hitscan, 에너지 소모 시스템
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UBeamWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

public:
	/*~ UWeaponInstance Interface ~*/
	virtual bool CanFire() const override;

	// 발사 중 여부
	UFUNCTION(BlueprintCallable, Category = "Weapon|Beam")
	bool IsFiring() const { return bIsFiring; }

	// 빔 시작
	UFUNCTION(BlueprintCallable, Category = "Weapon|Beam")
	void StartFiring();

	// 빔 종료
	UFUNCTION(BlueprintCallable, Category = "Weapon|Beam")
	void StopFiring();

	// 현재 에너지 조회
	UFUNCTION(BlueprintCallable, Category = "Weapon|Beam")
	float GetCurrentEnergy() const;

	// 에너지 소모 (DeltaTime 기반)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Beam")
	void ConsumeEnergy(float DeltaTime);

protected:
	// 발사 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Beam")
	bool bIsFiring = false;
};
