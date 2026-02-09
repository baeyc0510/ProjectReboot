// PRGCN_SlowMotion.cpp
#include "PRGCN_SlowMotion.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

APRGCN_SlowMotion::APRGCN_SlowMotion()
{
	// 태그 자동 매칭
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.SlowMotion"));

	// GCN_Actor는 기본적으로 자동 파괴 비활성화
	bAutoDestroyOnRemove = true;
}

bool APRGCN_SlowMotion::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!IsValid(MyTarget))
	{
		return false;
	}

	CachedTargetActor = MyTarget;

	ApplySlowMotion(MyTarget);
	ApplyPostProcess(MyTarget);

	return true;
}

bool APRGCN_SlowMotion::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	AActor* TargetActor = CachedTargetActor.Get();
	if (!IsValid(TargetActor))
	{
		TargetActor = MyTarget;
	}

	RestoreNormalTime(TargetActor);
	RestorePostProcess();

	return true;
}

void APRGCN_SlowMotion::ApplySlowMotion(AActor* TargetActor)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 원래 값 저장
	OriginalGlobalTimeDilation = World->GetWorldSettings()->TimeDilation;
	if (IsValid(TargetActor))
	{
		OriginalPlayerTimeDilation = TargetActor->CustomTimeDilation;
	}

	// 글로벌 타임 딜레이션 적용 (월드 슬로우)
	UGameplayStatics::SetGlobalTimeDilation(World, GlobalTimeDilation);

	// 플레이어 타임 딜레이션 보정 (정상 속도 유지)
	if (IsValid(TargetActor) && GlobalTimeDilation > 0.0f)
	{
		// 글로벌 딜레이션을 상쇄하여 정상 속도 유지
		float CompensatedDilation = PlayerTimeDilationMultiplier / GlobalTimeDilation;
		TargetActor->CustomTimeDilation = CompensatedDilation;
	}
}

void APRGCN_SlowMotion::RestoreNormalTime(AActor* TargetActor)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 글로벌 타임 딜레이션 복원
	UGameplayStatics::SetGlobalTimeDilation(World, OriginalGlobalTimeDilation);

	// 플레이어 타임 딜레이션 복원
	if (IsValid(TargetActor))
	{
		TargetActor->CustomTimeDilation = OriginalPlayerTimeDilation;
	}
}

void APRGCN_SlowMotion::ApplyPostProcess(AActor* TargetActor)
{
	if (!SlowMotionLUT)
	{
		return;
	}

	if (!IsValid(TargetActor))
	{
		return;
	}

	UCameraComponent* CameraComp = TargetActor->FindComponentByClass<UCameraComponent>();
	if (!IsValid(CameraComp))
	{
		return;
	}

	CachedCameraComponent = CameraComp;
	OriginalPostProcessSettings = CameraComp->PostProcessSettings;
	OriginalPostProcessBlendWeight = CameraComp->PostProcessBlendWeight;

	CameraComp->PostProcessBlendWeight = 1.0f;
	CameraComp->PostProcessSettings.bOverride_ColorGradingLUT = true;
	CameraComp->PostProcessSettings.bOverride_ColorGradingIntensity = true;
	CameraComp->PostProcessSettings.ColorGradingLUT = SlowMotionLUT;
	CameraComp->PostProcessSettings.ColorGradingIntensity = SlowMotionLUTIntensity;

	bPostProcessApplied = true;
}

void APRGCN_SlowMotion::RestorePostProcess()
{
	if (!bPostProcessApplied)
	{
		return;
	}

	UCameraComponent* CameraComp = CachedCameraComponent.Get();
	if (!IsValid(CameraComp))
	{
		return;
	}

	CameraComp->PostProcessSettings = OriginalPostProcessSettings;
	CameraComp->PostProcessBlendWeight = OriginalPostProcessBlendWeight;

	bPostProcessApplied = false;
	CachedCameraComponent.Reset();
}
