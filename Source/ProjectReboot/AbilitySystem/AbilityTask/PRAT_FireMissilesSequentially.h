// PRAT_FireMissilesSequentially.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "PRAT_FireMissilesSequentially.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireNextMissile, AActor*, HomingTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMissilesFired);

/**
 * 미사일 순차 발사 AbilityTask
 * AvatarActor의 CustomTimeDilation을 반영하여 일정 간격으로 미사일을 발사한다.
 */
UCLASS()
class PROJECTREBOOT_API UPRAT_FireMissilesSequentially : public UAbilityTask
{
	GENERATED_BODY()

public:
	/// 순차 발사 태스크 생성
	static UPRAT_FireMissilesSequentially* CreateTask(
		UGameplayAbility* OwningAbility,
		const TArray<AActor*>& InTargets,
		float InFireInterval = 0.1f);

	/// 다음 미사일 발사 시점마다 호출
	UPROPERTY(BlueprintAssignable)
	FOnFireNextMissile OnFireNextMissile;

	/// 모든 미사일 발사 완료 시 호출
	UPROPERTY(BlueprintAssignable)
	FOnAllMissilesFired OnAllMissilesFired;

	/*~ UAbilityTask Interface ~*/
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	// 발사 대상 목록
	TArray<TWeakObjectPtr<AActor>> Targets;

	// 발사 간격 (AvatarActor 시간 기준)
	float FireInterval;

	// 누적 시간
	float AccumulatedTime;

	// 현재 발사 인덱스
	int32 CurrentIndex;
};
