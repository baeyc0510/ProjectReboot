// PRCameraComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PRCameraConfig.h"
#include "PRCameraComponent.generated.h"

class USpringArmComponent;

UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UPRCameraComponent();

	virtual void OnComponentCreated() override;
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void SetTargetSetting(const FPRCameraSetting& InSetting, float Duration, UCurveFloat* TransitionCurve = nullptr);
	void SetSettingImmediate(const FPRCameraSetting& InSetting);
	void ResetToDefault(float Duration, UCurveFloat* TransitionCurve = nullptr);

	UPRCameraConfig* GetConfig() const { return Config; }
	float GetCurrentSensitivityMultiplier() const { return CurrentSensitivity; }

protected:
	virtual void BeginPlay() override;
	
private:
	void InitializeCameraRig();
	void UpdateBlend(float DeltaTime);
	void ApplyToSpringArm(float Alpha);
	FPRCameraSetting GetCurrentBlendedSetting() const;
	
protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Camera")
	TObjectPtr<UPRCameraConfig> Config;

#if WITH_EDITORONLY_DATA
	/** 에디터 뷰포트에서 특정 상태의 카메라 배치를 미리보기 위한 태그 */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FGameplayTag PreviewStateTag;
#endif
	
private:
	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> ParentSpringArm;

	bool bIsTransitioning = false;
	float TransitionElapsed = 0.0f;
	float TransitionDuration = 0.3f;
	float TransitionProgress = 0.0f;
	float CurrentFOV = 90.0f;
	float CurrentSensitivity = 1.0f;

	FPRCameraSetting FromSetting;
	FPRCameraSetting ToSetting;

	UPROPERTY()
	TObjectPtr<UCurveFloat> CurrentTransitionCurve;
};