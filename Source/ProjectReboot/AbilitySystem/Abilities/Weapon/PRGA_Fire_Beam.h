// PRGA_Fire_Beam.h
#pragma once

#include "CoreMinimal.h"
#include "PRGameplayAbility_WeaponFire.h"
#include "PRGA_Fire_Beam.generated.h"

class UBeamWeaponInstance;

/**
 * 빔 무기 발사 어빌리티 (지속형 Hitscan)
 * 입력 유지 동안 지속 발사, 에너지 소모
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_Fire_Beam : public UPRGameplayAbility_WeaponFire
{
	GENERATED_BODY()

public:
	UPRGA_Fire_Beam();

	/*~ UGameplayAbility Interface ~*/
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	/*~ UPRGameplayAbility_WeaponFire Interface ~*/
	virtual void OnActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	// Tick 처리 (빔 업데이트)
	void TickBeam(float DeltaTime);

	// BeamWeaponInstance 조회
	UBeamWeaponInstance* GetBeamWeapon() const;

protected:
	// 데미지 적용 간격 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam")
	float DamageInterval = 0.1f;

private:
	// 타이머 핸들
	FTimerHandle BeamTickTimerHandle;

	// 마지막 데미지 적용 시간
	float LastDamageTime = 0.0f;
};
