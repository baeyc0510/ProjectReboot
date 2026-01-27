// PRLockOnViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PRLockOnViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnVisibilityChanged, bool, bIsVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnProgressChanged, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnStateChanged, bool, bIsLocked);

/**
 * 락온 대상별 상태 뷰모델
 * - PRTargetLockComponent에서 타겟별로 생성/관리
 * - 진행률(0~1)과 락온 완료 여부를 UI에 노출
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRLockOnViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	/*~ UPRViewModelBase Interface ~*/
	virtual void InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer) override;
	virtual void Deinitialize() override;

public:
	/*~ UPRLockOnViewModel Interface ~*/

	// 가시성 반환
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsVisible() const { return bIsVisible; }

	// 락온 진행률 반환 (0~1)
	UFUNCTION(BlueprintPure, Category = "LockOn")
	float GetProgress() const { return Progress; }

	// 락온 완료 여부 반환
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLocked() const { return bIsLocked; }

	// 가시성 설정 (PRTargetLockComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetVisible(bool bNewVisible);

	// 진행률 설정 (PRTargetLockComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetProgress(float NewProgress);

	// 락온 완료 상태 설정 (PRTargetLockComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetLocked(bool bNewIsLocked);

public:
	// 가시성 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Events")
	FOnLockOnVisibilityChanged OnVisibilityChanged;

	// 진행률 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Events")
	FOnLockOnProgressChanged OnProgressChanged;

	// 락온 완료/해제 이벤트
	UPROPERTY(BlueprintAssignable, Category = "LockOn|Events")
	FOnLockOnStateChanged OnLockedStateChanged;

private:
	// 가시성 여부
	bool bIsVisible = false;

	// 현재 락온 진행률 (0~1)
	float Progress = 0.0f;

	// 락온 완료 여부
	bool bIsLocked = false;
};
