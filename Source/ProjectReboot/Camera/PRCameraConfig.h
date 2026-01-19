// PRCameraConfig.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PRCameraConfig.generated.h"

class UCurveFloat;

/**
 * 카메라 개별 설정
 */
USTRUCT(BlueprintType)
struct FPRCameraSetting
{
	GENERATED_BODY()

	// SpringArm 길이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TargetArmLength = 400.0f;

	// 카메라 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector SocketOffset = FVector::ZeroVector;

	// 타겟 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector TargetOffset = FVector::ZeroVector;

	// 카메라 FOV
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FieldOfView = 90.0f;

	// 카메라 위치 Lag 사용 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEnableCameraLag = true;

	// 카메라 위치 Lag 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CameraLagSpeed = 10.0f;

	// 카메라 회전 Lag 사용 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEnableCameraRotationLag = false;

	// 카메라 회전 Lag 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CameraRotationLagSpeed = 10.0f;
	
	// 카메라 감도 배율 (기본값 1.0, ADS 시 0.5 등으로 조절)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.05", ClampMax = "2.0"))
	float SensitivityMultiplier = 1.0f;
};

/**
 * 태그별 카메라 설정 (우선순위 포함)
 */
USTRUCT(BlueprintType)
struct FPRCameraTagSetting
{
	GENERATED_BODY()

	// 이 설정이 적용될 상태 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ConditionTags;

	// 우선순위 (높을수록 우선)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Priority = 0;

	// 카메라 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FPRCameraSetting CameraSetting;

	// 전환 시 사용할 커브 (nullptr이면 SmoothStep)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCurveFloat> TransitionCurve;

	// 전환 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TransitionDuration = 0.3f;
};

/**
 * 인라인 편집이 가능한 카메라 설정
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECTREBOOT_API UPRCameraConfig : public UObject
{
	GENERATED_BODY()

public:
	// 태그별 카메라 설정 배열
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TArray<FPRCameraTagSetting> CameraSettings;

	// 기본 카메라 설정 (태그 없을 때)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FPRCameraSetting DefaultSetting;

	// 기본 전환 시간 (Default로 복귀 시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float DefaultTransitionDuration = 0.3f;

	// 기본 전환 커브 (Default로 복귀 시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCurveFloat> DefaultTransitionCurve;

	// 현재 활성 태그 중 가장 높은 우선순위 설정 반환
	const FPRCameraTagSetting* GetHighestPrioritySetting(const FGameplayTagContainer& ActiveTags) const;
};
