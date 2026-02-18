// PRGCN_SlowMotion.cpp
#include "PRGCN_SlowMotion.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

APRGCN_SlowMotion::APRGCN_SlowMotion()
{
	// 페이드 아웃 완료 후 수동 파괴
	bAutoDestroyOnRemove = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void APRGCN_SlowMotion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bPostProcessApplied)
	{
		return;
	}

	const float TargetIntensity = bFadingOut ? 0.0f : SlowMotionLUTIntensity;

	// 이미 목표 도달
	if (FMath::IsNearlyEqual(CurrentLUTIntensity, TargetIntensity, KINDA_SMALL_NUMBER))
	{
		CurrentLUTIntensity = TargetIntensity;

		if (bFadingOut)
		{
			RestorePostProcess();
			SetActorTickEnabled(false);
			Destroy();
		}
		return;
	}

	// 실시간 델타 사용 (타임 딜레이션 영향 없이 일정한 보간 속도)
	const float RealDelta = FApp::GetDeltaTime();
	const float FadeDuration = bFadingOut ? LUTFadeOutDuration : LUTFadeInDuration;
	const float InterpSpeed = (FadeDuration > 0.0f) ? (SlowMotionLUTIntensity / FadeDuration) : 1000.0f;

	CurrentLUTIntensity = FMath::FInterpConstantTo(CurrentLUTIntensity, TargetIntensity, RealDelta, InterpSpeed);

	UCameraComponent* CameraComp = CachedCameraComponent.Get();
	if (IsValid(CameraComp))
	{
		CameraComp->PostProcessSettings.ColorGradingIntensity = CurrentLUTIntensity;
	}
}

bool APRGCN_SlowMotion::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!IsValid(MyTarget))
	{
		return false;
	}

	CachedTargetActor = MyTarget;

	ApplySlowMotion(MyTarget);
	SetupPostProcess(MyTarget);

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

	// LUT 페이드 아웃 시작 (Tick에서 보간 후 파괴)
	bFadingOut = true;

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

	UGameplayStatics::SetGlobalTimeDilation(World, OriginalGlobalTimeDilation);

	if (IsValid(TargetActor))
	{
		TargetActor->CustomTimeDilation = OriginalPlayerTimeDilation;
	}
}

void APRGCN_SlowMotion::SetupPostProcess(AActor* TargetActor)
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

	// LUT 바인딩하되 강도 0에서 시작 (Tick에서 보간)
	CameraComp->PostProcessBlendWeight = 1.0f;
	CameraComp->PostProcessSettings.bOverride_ColorGradingLUT = true;
	CameraComp->PostProcessSettings.bOverride_ColorGradingIntensity = true;
	CameraComp->PostProcessSettings.ColorGradingLUT = SlowMotionLUT;
	CameraComp->PostProcessSettings.ColorGradingIntensity = 0.0f;

	CurrentLUTIntensity = 0.0f;
	bFadingOut = false;
	bPostProcessApplied = true;

	SetActorTickEnabled(true);
}

void APRGCN_SlowMotion::RestorePostProcess()
{
	if (!bPostProcessApplied)
	{
		return;
	}

	UCameraComponent* CameraComp = CachedCameraComponent.Get();
	if (IsValid(CameraComp))
	{
		CameraComp->PostProcessSettings = OriginalPostProcessSettings;
		CameraComp->PostProcessBlendWeight = OriginalPostProcessBlendWeight;
	}

	bPostProcessApplied = false;
	CachedCameraComponent.Reset();
}
