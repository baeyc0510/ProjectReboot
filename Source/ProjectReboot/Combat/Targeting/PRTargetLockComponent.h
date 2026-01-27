// PRTargetLockComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/Character/PRCharacterBase.h"
#include "PRTargetLockComponent.generated.h"

class APRCharacterBase;
class UMissileWeaponInstance;
class UPREquipmentManagerComponent;
class UPRLockOnViewModel;
class ULocalPlayer;
class UAbilitySystemComponent;

// 락온 진행 중인 타겟 정보
USTRUCT()
struct FLockingTarget
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	UPROPERTY()
	TWeakObjectPtr<UPRLockOnViewModel> ViewModel;

	// 타겟의 ASC (태그 이벤트 바인딩용)
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;

	// 태그 변경 이벤트 델리게이트 핸들
	FDelegateHandle TagEventHandle;

	// 락온 진행 시간
	float LockingTime = 0.0f;

	// 락온 완료 여부
	bool bIsLocked = false;

	// 조건 실패 후 경과 시간 (유예 시간 추적)
	float OutOfConditionTime = 0.0f;

	// 현재 조건 충족 여부
	bool bIsInCondition = true;
};

/**
 * 타겟 락온 컴포넌트
 * - GA_LockOn 획득 시 동적으로 부착
 * - 범위/시야각 내 락온 가능 대상 탐색
 * - 락온된 타겟을 MissileWeaponInstance에 전달
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPRTargetLockComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 컴포넌트 활성화/비활성화
	UFUNCTION(BlueprintCallable, Category = "TargetLock")
	void SetLockOnEnabled(bool bEnabled);

	// 현재 락온 완료된 타겟 목록
	UFUNCTION(BlueprintCallable, Category = "TargetLock")
	TArray<AActor*> GetLockedTargets() const;

	// 현재 락온 진행 중인 타겟 목록
	UFUNCTION(BlueprintCallable, Category = "TargetLock")
	TArray<AActor*> GetLockingTargets() const;

	// 특정 타겟의 락온 진행률 (0~1)
	UFUNCTION(BlueprintCallable, Category = "TargetLock")
	float GetLockProgress(AActor* Target) const;

	// 모든 락온 해제
	UFUNCTION(BlueprintCallable, Category = "TargetLock")
	void ClearAllLocks();

	/*~ 설정값 Override 함수들 ~*/

	UFUNCTION(BlueprintCallable, Category = "TargetLock|Override")
	void OverrideLockOnRange(float NewRange) { LockOnRange = NewRange; }

	UFUNCTION(BlueprintCallable, Category = "TargetLock|Override")
	void OverrideLockOnConeAngle(float NewAngle) { LockOnConeAngle = NewAngle; }

	UFUNCTION(BlueprintCallable, Category = "TargetLock|Override")
	void OverrideLockOnTime(float NewTime) { LockOnTime = NewTime; }

	// 현재 최대 락온 가능 수 (장착된 미사일 수 기반)
	UFUNCTION(BlueprintPure, Category = "TargetLock")
	int32 GetMaxLockCount() const;

	UFUNCTION(BlueprintCallable, Category = "TargetLock|Override")
	void OverrideLockableTargetTag(FGameplayTag NewTag) { LockableTargetTag = NewTag; }

	UFUNCTION(BlueprintCallable, Category = "TargetLock|Override")
	void OverrideTargetFilterClass(TSubclassOf<AActor> NewClass) { TargetFilterClass = NewClass; }

protected:
	virtual void BeginPlay() override;

	// 장비 매니저 캐시
	void CacheEquipmentManager();

	// 범위 내 락온 가능한 타겟 탐색
	TArray<AActor*> FindLockableTargets() const;

	// 타겟이 락온 가능한지 확인 (태그, 시야 등)
	bool CanLockTarget(AActor* Target) const;

	// 타겟이 시야각 내에 있는지 확인
	bool IsTargetInViewCone(AActor* Target) const;

	// 타겟에 대한 시야가 확보되어 있는지 (Line of Sight)
	bool HasLineOfSightTo(AActor* Target) const;

	// 타겟의 시야 체크용 위치들 반환 (머리/몸통/발)
	TArray<FVector> GetTargetCheckLocations(AActor* Target) const;

	// 락온 진행 업데이트
	void UpdateLockingTargets(float DeltaTime);

	// WeaponInstance에 락온 타겟 동기화
	void SyncLockedTargetsToWeapon();

	// 현재 미사일 무기 인스턴스 조회
	UMissileWeaponInstance* GetMissileWeaponInstance() const;

	// 카메라 뷰 정보 조회
	void GetCameraViewInfo(FVector& OutLocation, FRotator& OutRotation) const;

	// 타겟에 대한 ViewModel 조회 또는 생성
	UPRLockOnViewModel* GetOrCreateLockOnViewModel(AActor* Target);

	// 소유 LocalPlayer 조회
	ULocalPlayer* GetLocalPlayer() const;

	// 타겟에 태그 이벤트 바인딩
	void BindTargetTagEvent(FLockingTarget& LockingTarget);

	// 타겟의 태그 이벤트 언바인딩
	void UnbindTargetTagEvent(FLockingTarget& LockingTarget);

	// 락온 대상 태그 제거 시 호출
	void HandleLockableTagRemoved(AActor* Target);

protected:
	// 락온 활성화 여부
	UPROPERTY(BlueprintReadOnly, Category = "TargetLock")
	bool bIsLockOnEnabled = false;

	// 락온 대상 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	TSubclassOf<AActor> TargetFilterClass = APRCharacterBase::StaticClass();
	
	// 락온 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	float LockOnRange = 3000.0f;

	// 락온 시야각 (도, 전체 각도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	float LockOnConeAngle = 60.0f;

	// 락온 소요 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	float LockOnTime = 0.8f;

	// 락온 대상 판별 태그 (이 태그를 가진 액터만 락온 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	FGameplayTag LockableTargetTag;

	// 조건 미충족 시 유예 시간 (이 시간 동안은 락온 유지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TargetLock|Settings")
	float LockGracePeriod = 0.3f;

	// 현재 락온 진행 중인 타겟들
	UPROPERTY()
	TArray<FLockingTarget> LockingTargets;

	// 장비 매니저 캐시
	UPROPERTY()
	TWeakObjectPtr<UPREquipmentManagerComponent> CachedEquipmentManager;
};
