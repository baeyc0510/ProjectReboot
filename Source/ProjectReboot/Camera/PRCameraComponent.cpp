// PRCameraComponent.cpp
#include "PRCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Curves/CurveFloat.h"

UPRCameraComponent::UPRCameraComponent()
{
	bUsePawnControlRotation = false;
}

void UPRCameraComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	InitializeCameraRig();
}

void UPRCameraComponent::OnRegister()
{
	Super::OnRegister();
	InitializeCameraRig();
}

void UPRCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeCameraRig();
}

void UPRCameraComponent::InitializeCameraRig()
{
	if (!ParentSpringArm)
	{
		ParentSpringArm = Cast<USpringArmComponent>(GetAttachParent());
	}

	// 실제 스프링암의 현재 수치를 내부 데이터에 동기화
	if (ParentSpringArm)
	{
		ToSetting.TargetArmLength = ParentSpringArm->TargetArmLength;
		ToSetting.SocketOffset = ParentSpringArm->SocketOffset;
		ToSetting.TargetOffset = ParentSpringArm->TargetOffset;
		// 필요 시 다른 변수들도 동기화

		// FromSetting도 현재 위치로 맞춤
		FromSetting = ToSetting;
	}
}

#if WITH_EDITOR
void UPRCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		                           ? PropertyChangedEvent.Property->GetFName()
		                           : NAME_None;

	// 변경된 프로퍼티가 Config나 PreviewStateTag와 관련된 것인지 확인
	const bool bConfigChanged = PropertyName == GET_MEMBER_NAME_CHECKED(UPRCameraComponent, Config);
	const bool bPreviewChanged = PropertyName == GET_MEMBER_NAME_CHECKED(UPRCameraComponent, PreviewStateTag);
	const bool bMemberChanged = PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetFName()
		== GET_MEMBER_NAME_CHECKED(UPRCameraComponent, Config);

	if (bConfigChanged || bPreviewChanged || bMemberChanged)
	{
		InitializeCameraRig();
		if (!IsValid(Config))
		{
			return;
		}

		FGameplayTagContainer PreviewContainer;
		if (PreviewStateTag.IsValid())
		{
			PreviewContainer.AddTag(PreviewStateTag);
		}

		// 런타임과 동일한 우선순위 로직으로 설정 검색
		const FPRCameraTagSetting* BestSetting = Config->GetHighestPrioritySetting(PreviewContainer);

		// 결과 적용 (검색 결과가 없으면 DefaultSetting 적용)
		if (BestSetting)
		{
			SetSettingImmediate(BestSetting->CameraSetting);
		}
		else
		{
			SetSettingImmediate(Config->DefaultSetting);
		}
	}
}
#endif

void UPRCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	if (bIsTransitioning)
	{
		UpdateBlend(DeltaTime);
	}
	Super::GetCameraView(DeltaTime, DesiredView);

	float FOVOffset = DesiredView.FOV - FieldOfView;
	DesiredView.FOV = CurrentFOV + FOVOffset;
}

void UPRCameraComponent::SetTargetSetting(const FPRCameraSetting& InSetting, float Duration,
                                          UCurveFloat* TransitionCurve)
{
	FromSetting = GetCurrentBlendedSetting();
	ToSetting = InSetting;
	TransitionDuration = FMath::Max(Duration, 0.0f);
	CurrentTransitionCurve = TransitionCurve;
	TransitionElapsed = 0.0f;
	bIsTransitioning = true;

	if (TransitionDuration <= 0.0f)
	{
		SetSettingImmediate(InSetting);
	}
}

void UPRCameraComponent::SetSettingImmediate(const FPRCameraSetting& InSetting)
{
	FromSetting = ToSetting = InSetting;
	TransitionProgress = 1.0f;
	bIsTransitioning = false;
	CurrentFOV = InSetting.FieldOfView;
	CurrentSensitivity = InSetting.SensitivityMultiplier;
	ApplyToSpringArm(1.0f);
}

void UPRCameraComponent::ResetToDefault(float Duration, UCurveFloat* TransitionCurve)
{
	if (IsValid(Config))
	{
		SetTargetSetting(Config->DefaultSetting, Duration, TransitionCurve);
	}
}

void UPRCameraComponent::UpdateBlend(float DeltaTime)
{
	TransitionElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);
	TransitionProgress = IsValid(CurrentTransitionCurve)
		                     ? CurrentTransitionCurve->GetFloatValue(Alpha)
		                     : FMath::SmoothStep(0.0f, 1.0f, Alpha);

	ApplyToSpringArm(TransitionProgress);
	if (Alpha >= 1.0f) { bIsTransitioning = false; }
}

void UPRCameraComponent::ApplyToSpringArm(float Alpha)
{
	if (!IsValid(ParentSpringArm)) return;

	ParentSpringArm->TargetArmLength = FMath::Lerp(FromSetting.TargetArmLength, ToSetting.TargetArmLength, Alpha);
	ParentSpringArm->SocketOffset = FMath::Lerp(FromSetting.SocketOffset, ToSetting.SocketOffset, Alpha);
	ParentSpringArm->TargetOffset = FMath::Lerp(FromSetting.TargetOffset, ToSetting.TargetOffset, Alpha);

	const FPRCameraSetting& ActiveSource = (Alpha < 0.5f) ? FromSetting : ToSetting;
	ParentSpringArm->bEnableCameraLag = ActiveSource.bEnableCameraLag;
	ParentSpringArm->CameraLagSpeed = FMath::Lerp(FromSetting.CameraLagSpeed, ToSetting.CameraLagSpeed, Alpha);
	ParentSpringArm->bEnableCameraRotationLag = ActiveSource.bEnableCameraRotationLag;
	ParentSpringArm->CameraRotationLagSpeed = FMath::Lerp(FromSetting.CameraRotationLagSpeed,
	                                                      ToSetting.CameraRotationLagSpeed, Alpha);

	CurrentFOV = FMath::Lerp(FromSetting.FieldOfView, ToSetting.FieldOfView, Alpha);
	CurrentSensitivity = FMath::Lerp(FromSetting.SensitivityMultiplier, ToSetting.SensitivityMultiplier, Alpha);
}

FPRCameraSetting UPRCameraComponent::GetCurrentBlendedSetting() const
{
	if (!bIsTransitioning)
	{
		return ToSetting;
	}

	FPRCameraSetting Blended = ToSetting;
	Blended.TargetArmLength = FMath::Lerp(FromSetting.TargetArmLength, ToSetting.TargetArmLength, TransitionProgress);
	Blended.SocketOffset = FMath::Lerp(FromSetting.SocketOffset, ToSetting.SocketOffset, TransitionProgress);
	Blended.TargetOffset = FMath::Lerp(FromSetting.TargetOffset, ToSetting.TargetOffset, TransitionProgress);
	Blended.FieldOfView = FMath::Lerp(FromSetting.FieldOfView, ToSetting.FieldOfView, TransitionProgress);
	Blended.SensitivityMultiplier = FMath::Lerp(FromSetting.SensitivityMultiplier, ToSetting.SensitivityMultiplier,
	                                            TransitionProgress);

	const FPRCameraSetting& ActiveSource = (TransitionProgress < 0.5f) ? FromSetting : ToSetting;
	Blended.bEnableCameraLag = ActiveSource.bEnableCameraLag;
	Blended.CameraLagSpeed = FMath::Lerp(FromSetting.CameraLagSpeed, ToSetting.CameraLagSpeed, TransitionProgress);
	Blended.bEnableCameraRotationLag = ActiveSource.bEnableCameraRotationLag;
	Blended.CameraRotationLagSpeed = FMath::Lerp(FromSetting.CameraRotationLagSpeed, ToSetting.CameraRotationLagSpeed,
	                                             TransitionProgress);

	return Blended;
}
