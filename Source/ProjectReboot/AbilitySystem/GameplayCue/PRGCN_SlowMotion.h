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
 * - PRGA_SlowMotion이 AddGameplayCue/RemoveGameplayCue로 라이프사이클 관리
 * - 타임 딜레이션은 즉시 적용/해제
 * - LUT 포스트 프로세스는 페이드 인/아웃 보간
 */
UCLASS()
class PROJECTREBOOT_API APRGCN_SlowMotion : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	APRGCN_SlowMotion();

	/*~ AActor Interface ~*/
	virtual void Tick(float DeltaTime) override;

	/*~ AGameplayCueNotify_Actor Interface ~*/
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	// 슬로우 모션 적용
	void ApplySlowMotion(AActor* TargetActor);

	// 슬로우 모션 해제
	void RestoreNormalTime(AActor* TargetActor);

	// 포스트 프로세스 초기 설정 (LUT 바인딩, 강도 0에서 시작)
	void SetupPostProcess(AActor* TargetActor);

	// 포스트 프로세스 원본 복원
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

	// 슬로우 모션 LUT 목표 강도 (0.0 ~ 1.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion|PostProcess", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowMotionLUTIntensity = 1.0f;

	// LUT 페이드 인 시간 (초, 실시간 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion|PostProcess", meta = (ClampMin = "0.0"))
	float LUTFadeInDuration = 0.15f;

	// LUT 페이드 아웃 시간 (초, 실시간 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowMotion|PostProcess", meta = (ClampMin = "0.0"))
	float LUTFadeOutDuration = 0.15f;

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

	// 현재 LUT 보간 강도
	float CurrentLUTIntensity = 0.0f;

	// 포스트 프로세스 적용 여부
	bool bPostProcessApplied = false;

	// 페이드 아웃 진행 중 여부
	bool bFadingOut = false;
};
