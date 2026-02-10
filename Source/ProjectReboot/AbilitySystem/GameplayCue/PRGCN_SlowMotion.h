// PRGCN_SlowMotion.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "Engine/EngineTypes.h"
#include "PRGCN_SlowMotion.generated.h"

class UCameraComponent;
class UTexture;

/**
 * 슬로우 모션 GameplayCue (지속형)
 * - GE 지속 시간 동안 글로벌 타임 딜레이션 적용
 * - 타겟(플레이어)은 CustomTimeDilation으로 정상 속도 유지
 * - 포스트 프로세스 LUT로 비주얼 연출
 *
 * GE Stacking 설정 (Duration 갱신만 되도록):
 * - Stacking Type: Aggregate by Target
 * - Stack Limit Count: 1
 * - Stack Duration Refresh Policy: Refresh on Successful Application
 * - Stack Expiration Policy: Clear Entire Stack
 *
 * 콜백 동작:
 * - 최초 GE 적용 시: OnActive 호출 (슬로모션 시작)
 * - Duration 갱신 시: OnActive 재호출 안됨 (GCN 이미 활성 상태)
 * - Duration 만료 시: OnRemove 호출 (슬로모션 종료)
 */
UCLASS()
class PROJECTREBOOT_API APRGCN_SlowMotion : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	APRGCN_SlowMotion();

	/*~ AGameplayCueNotify_Actor Interface ~*/
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	// 슬로우 모션 적용
	void ApplySlowMotion(AActor* TargetActor);

	// 슬로우 모션 해제
	void RestoreNormalTime(AActor* TargetActor);

	// 포스트 프로세스 적용
	void ApplyPostProcess(AActor* TargetActor);

	// 포스트 프로세스 해제
	void RestorePostProcess();

protected:
	// 글로벌 타임 딜레이션 (0.0 ~ 1.0, 낮을수록 느림)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion")
	float GlobalTimeDilation = 0.3f;

	// 플레이어 타임 딜레이션 보정 (1.0 = 정상 속도 유지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion")
	float PlayerTimeDilationMultiplier = 1.0f;

	// 슬로우 모션 컬러 그레이딩 LUT
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion|PostProcess")
	TObjectPtr<UTexture> SlowMotionLUT;

	// 슬로우 모션 LUT 강도 (0.0 ~ 1.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion|PostProcess", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowMotionLUTIntensity = 1.0f;

private:
	// 원래 글로벌 타임 딜레이션 저장
	float OriginalGlobalTimeDilation = 1.0f;

	// 원래 플레이어 타임 딜레이션 저장
	float OriginalPlayerTimeDilation = 1.0f;

	// 원래 포스트 프로세스 설정 저장
	FPostProcessSettings OriginalPostProcessSettings;

	// 원래 포스트 프로세스 블렌드 가중치 저장
	float OriginalPostProcessBlendWeight = 0.0f;

	// 포스트 프로세스 적용한 카메라 캐시
	TWeakObjectPtr<UCameraComponent> CachedCameraComponent;

	// 타겟 액터 캐시
	TWeakObjectPtr<AActor> CachedTargetActor;

	// 포스트 프로세스 적용 여부
	bool bPostProcessApplied = false;
};
