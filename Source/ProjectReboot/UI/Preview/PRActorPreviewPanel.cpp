// PRActorPreviewPanel.cpp
#include "PRActorPreviewPanel.h"
#include "Components/Image.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

UPRActorPreviewPanel::UPRActorPreviewPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);

	YawSettings.bEnabled = true;
	YawSettings.DragInput = EPRDragInput::DragX;
	YawSettings.Sensitivity = 0.5f;

	PitchSettings.bEnabled = false;
	PitchSettings.DragInput = EPRDragInput::DragY;
	PitchSettings.Sensitivity = 0.5f;
	PitchSettings.bClampAngle = true;
	PitchSettings.MinAngle = -89.f;
	PitchSettings.MaxAngle = 89.f;

	RollSettings.bEnabled = false;
}

void UPRActorPreviewPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// 미리보기 설정
	if (IsDesignTime())
	{
		if (Img_Preview)
		{
			FSlateBrush Brush;
			if (MaterialOverride)
			{
				Brush.SetResourceObject(MaterialOverride);
			}
			else if (RenderTargetOverride)
			{
				Brush.SetResourceObject(RenderTargetOverride);
			}

			Brush.ImageSize = FVector2D(RenderSettings.Resolution.X, RenderSettings.Resolution.Y);
			Img_Preview->SetBrush(Brush);
		}
		return;
	}

	CurrentRotation = InitialRotation;
	CurrentCameraDistance = CameraSettings.Distance;
	InitializePreviewScene();
}

void UPRActorPreviewPanel::NativeDestruct()
{
	DestroyPreviewScene();
	Super::NativeDestruct();
}

void UPRActorPreviewPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(PreviewActor))
	{
		return;
	}
	if (!AutoRotateSettings.bEnabled)
	{
		return;
	}
	// 직접 회전 중이면
	if (AutoRotateSettings.bPauseOnDrag && bIsDragging)
	{
		return;
	}

	double& CurrentAxisValue = GetRotationAxisRef(AutoRotateSettings.Axis);
	CurrentAxisValue += AutoRotateSettings.Speed * InDeltaTime;
	ApplyActorRotation();
}

FPRAxisSettings& UPRActorPreviewPanel::GetAxisSettingsRef(EPRRotationAxis Axis)
{
	switch (Axis)
	{
	case EPRRotationAxis::Pitch: return PitchSettings;
	case EPRRotationAxis::Yaw: return YawSettings;
	case EPRRotationAxis::Roll: return RollSettings;
	default: return YawSettings;
	}
}

double& UPRActorPreviewPanel::GetRotationAxisRef(EPRRotationAxis Axis)
{
	switch (Axis)
	{
	case EPRRotationAxis::Pitch: return CurrentRotation.Pitch;
	case EPRRotationAxis::Yaw: return CurrentRotation.Yaw;
	case EPRRotationAxis::Roll: return CurrentRotation.Roll;
	default: return CurrentRotation.Yaw;
	}
}

void UPRActorPreviewPanel::InitializePreviewScene()
{
	if (bIsInitialized)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RenderTarget = GetOrCreateRenderTarget();
	if (!RenderTarget)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SceneCaptureActor = World->SpawnActor<AActor>(AActor::StaticClass(), PreviewWorldLocation, FRotator::ZeroRotator,
	                                              SpawnParams);
	if (!SceneCaptureActor)
	{
		return;
	}

	USceneComponent* Root = NewObject<USceneComponent>(SceneCaptureActor, TEXT("Root"));
	SceneCaptureActor->SetRootComponent(Root);
	Root->RegisterComponent();

	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(SceneCaptureActor, TEXT("SceneCapture"));
	SceneCaptureComponent->RegisterComponent();
	SceneCaptureComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	SceneCaptureComponent->TextureTarget = RenderTarget;

	// 프리뷰 전용 조명 생성 (Lighting Channel 1 사용하여 분리)
	PreviewLightComponent = NewObject<UDirectionalLightComponent>(SceneCaptureActor, TEXT("PreviewLight"));
	PreviewLightComponent->RegisterComponent();
	PreviewLightComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	PreviewLightComponent->SetRelativeRotation(LightSettings.Rotation);
	PreviewLightComponent->SetIntensity(LightSettings.Intensity);
	PreviewLightComponent->SetLightColor(LightSettings.LightColor);
	// Lighting Channel 1만 사용 (월드 오브젝트와 분리)
	PreviewLightComponent->SetLightingChannels(false, true, false);

	ConfigureSceneCapture(SceneCaptureComponent);
	UpdateCameraTransform();
	ApplyToImage(RenderTarget);

	bIsInitialized = true;
}

void UPRActorPreviewPanel::DestroyPreviewScene()
{
	ClearPreview();

	if (SceneCaptureActor)
	{
		SceneCaptureActor->Destroy();
		SceneCaptureActor = nullptr;
		SceneCaptureComponent = nullptr;
		PreviewLightComponent = nullptr;
	}

	if (RenderTarget && !RenderTargetOverride)
	{
		RenderTarget->ReleaseResource();
		RenderTarget = nullptr;
	}

	DynamicMaterial = nullptr;
	bIsInitialized = false;
}

UTextureRenderTarget2D* UPRActorPreviewPanel::GetOrCreateRenderTarget()
{
	if (RenderTargetOverride)
	{
		return DuplicateObject<UTextureRenderTarget2D>(RenderTargetOverride, this);
	}

	UTextureRenderTarget2D* NewTarget = NewObject<UTextureRenderTarget2D>(this);
	NewTarget->RenderTargetFormat = RenderSettings.Format;
	NewTarget->ClearColor = RenderSettings.ClearColor;
	NewTarget->InitCustomFormat(
		RenderSettings.Resolution.X,
		RenderSettings.Resolution.Y,
		PF_B8G8R8A8,
		!RenderSettings.bUseSRGB
	);
	NewTarget->UpdateResourceImmediate(true);

	return NewTarget;
}

void UPRActorPreviewPanel::ConfigureSceneCapture(USceneCaptureComponent2D* CaptureComponent)
{
	if (!CaptureComponent)
	{
		return;
	}

	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	// MaterialOverride 사용 시 알파 채널 캡처 (투명 배경 지원)
	CaptureComponent->CaptureSource = MaterialOverride
		? ESceneCaptureSource::SCS_SceneColorHDR
		: ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->FOVAngle = CameraSettings.FOV;
	CaptureComponent->bCaptureEveryFrame = true;
	CaptureComponent->bCaptureOnMovement = false;

	CaptureComponent->ShowFlags.SetAtmosphere(false);
	CaptureComponent->ShowFlags.SetFog(false);
	CaptureComponent->ShowFlags.SetVolumetricFog(false);
	CaptureComponent->ShowFlags.SetMotionBlur(false);
	CaptureComponent->ShowFlags.SetBloom(false);
	CaptureComponent->ShowFlags.SetEyeAdaptation(false);
}

void UPRActorPreviewPanel::ApplyToImage(UTexture* Texture)
{
	if (!Img_Preview || !Texture)
	{
		return;
	}

	UObject* BrushResource = nullptr;
	if (MaterialOverride)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialOverride, this);
		DynamicMaterial->SetTextureParameterValue(TEXT("PreviewTexture"), Texture);
		BrushResource = DynamicMaterial;
	}
	else
	{
		BrushResource = Texture;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(BrushResource);
	Brush.ImageSize = FVector2D(RenderSettings.Resolution.X, RenderSettings.Resolution.Y);

	Img_Preview->SetBrush(Brush);
}

void UPRActorPreviewPanel::UpdateCameraTransform()
{
	if (!SceneCaptureComponent)
	{
		return;
	}

	// 카메라 위치 계산: 액터 정면 앞에서 액터를 마주봄
	// 기본 오프셋: +X 방향(정면 앞)으로 Distance만큼
	FVector CameraOffset = FVector(CurrentCameraDistance, 0.f, 0.f);

	// CameraSettings.Angle 적용 (Pitch: 위아래, Yaw: 좌우)
	CameraOffset = CameraSettings.Angle.RotateVector(CameraOffset);

	// InitialRotation으로 전체 기준점 회전
	CameraOffset = InitialRotation.RotateVector(CameraOffset);

	// 최종 위치 = 오프셋 + 추가 Offset
	const FVector FinalLocation = CameraOffset + CameraSettings.Offset;

	// 원점(액터 위치)을 바라보는 회전 계산
	const FRotator LookAtRotation = (-FinalLocation).Rotation();

	SceneCaptureComponent->SetRelativeLocation(FinalLocation);
	SceneCaptureComponent->SetRelativeRotation(LookAtRotation);
}

void UPRActorPreviewPanel::ApplyActorRotation()
{
	if (PreviewActor)
	{
		PreviewActor->SetActorRotation(CurrentRotation);
	}
}

float UPRActorPreviewPanel::ProcessAxisInput(const FPRAxisSettings& Settings, const FVector2D& Delta,
                                             float CurrentValue)
{
	if (!Settings.bEnabled)
	{
		return CurrentValue;
	}

	float Input = (Settings.DragInput == EPRDragInput::DragX) ? Delta.X : Delta.Y;
	if (Settings.bInvert) Input *= -1.f;

	float NewValue = CurrentValue + Input * Settings.Sensitivity;

	if (Settings.bClampAngle)
	{
		NewValue = FMath::Clamp(NewValue, Settings.MinAngle, Settings.MaxAngle);
	}

	return NewValue;
}

void UPRActorPreviewPanel::RegisterPreviewActor(AActor* Actor)
{
	if (!IsValid(Actor) || !SceneCaptureComponent)
	{
		return;
	}

	PreviewActor = Actor;
	SceneCaptureComponent->ShowOnlyActors.Add(Actor);

	// 프리뷰 조명과 같은 Lighting Channel 사용 (Channel 1)
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
	{
		if (PrimComp)
		{
			FLightingChannels Channels;
			Channels.bChannel0 = false;
			Channels.bChannel1 = true;
			Channels.bChannel2 = false;
			PrimComp->SetLightingChannels(Channels.bChannel0, Channels.bChannel1, Channels.bChannel2);
		}
	}

	// SceneCaptureActor를 PreviewActor 위치로 이동하여 카메라가 올바른 타겟을 바라보도록 함
	if (SceneCaptureActor)
	{
		SceneCaptureActor->SetActorLocation(Actor->GetActorLocation());
	}
	UpdateCameraTransform();

	OnPreviewActorSet(Actor);
}

void UPRActorPreviewPanel::UnregisterPreviewActor()
{
	if (!PreviewActor)
	{
		return;
	}

	OnPreviewActorCleared(PreviewActor);

	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->ShowOnlyActors.Remove(PreviewActor);
	}

	if (ActorOwnership == EPRActorOwnership::Owned)
	{
		PreviewActor->Destroy();
	}
	else
	{
		RestoreInstanceSettings(PreviewActor);
	}

	PreviewActor = nullptr;
	RestoreData.bIsValid = false;
}

void UPRActorPreviewPanel::ApplyInstanceSettings(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	RestoreData.OriginalLocation = Actor->GetActorLocation();
	RestoreData.OriginalRotation = Actor->GetActorRotation();
	RestoreData.bOriginalCollisionEnabled = Actor->GetActorEnableCollision();
	RestoreData.bOriginalHiddenInGame = Actor->IsHidden();
	RestoreData.bIsValid = true;

	if (InstanceSettings.bRelocateToPreviewLocation)
	{
		Actor->SetActorLocation(PreviewWorldLocation);
	}

	// 항상 InitialRotation 적용 (카메라가 Forward 기준 정면을 바라보므로)
	Actor->SetActorRotation(InitialRotation);
	CurrentRotation = InitialRotation;

	if (InstanceSettings.bDisableCollision)
	{
		Actor->SetActorEnableCollision(false);
	}

	if (InstanceSettings.bHideInGame)
	{
		Actor->SetActorHiddenInGame(true);
	}
}

void UPRActorPreviewPanel::RestoreInstanceSettings(AActor* Actor)
{
	if (!IsValid(Actor) || !RestoreData.bIsValid)
	{
		return;
	}

	if (InstanceSettings.bRelocateToPreviewLocation)
	{
		Actor->SetActorLocation(RestoreData.OriginalLocation);
	}

	// 원래 회전값 복원
	Actor->SetActorRotation(RestoreData.OriginalRotation);

	if (InstanceSettings.bDisableCollision)
	{
		Actor->SetActorEnableCollision(RestoreData.bOriginalCollisionEnabled);
	}

	if (InstanceSettings.bHideInGame)
	{
		Actor->SetActorHiddenInGame(RestoreData.bOriginalHiddenInGame);
	}
}

FReply UPRActorPreviewPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		LastMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(GetCachedWidget().ToSharedRef());
	}
	return FReply::Unhandled();
}

FReply UPRActorPreviewPanel::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply UPRActorPreviewPanel::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsDragging || !IsValid(PreviewActor))
	{
		return FReply::Unhandled();
	}

	const FVector2D CurrentPos = InMouseEvent.GetScreenSpacePosition();
	const FVector2D Delta = CurrentPos - LastMousePosition;

	CurrentRotation.Pitch = ProcessAxisInput(PitchSettings, Delta, CurrentRotation.Pitch);
	CurrentRotation.Yaw = ProcessAxisInput(YawSettings, Delta, CurrentRotation.Yaw);
	CurrentRotation.Roll = ProcessAxisInput(RollSettings, Delta, CurrentRotation.Roll);

	ApplyActorRotation();

	LastMousePosition = CurrentPos;
	return FReply::Handled();
}

FReply UPRActorPreviewPanel::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!ZoomSettings.bEnabled)
	{
		return FReply::Unhandled();
	}

	const float WheelDelta = InMouseEvent.GetWheelDelta();
	CurrentCameraDistance = FMath::Clamp(
		CurrentCameraDistance - WheelDelta * ZoomSettings.Sensitivity,
		CameraSettings.MinDistance,
		CameraSettings.MaxDistance
	);

	UpdateCameraTransform();
	return FReply::Handled();
}

void UPRActorPreviewPanel::SetPreviewActorByClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return;
	}

	ClearPreview();

	UWorld* World = GetWorld();
	if (!World || !SceneCaptureComponent)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, PreviewWorldLocation, InitialRotation, SpawnParams);
	if (!IsValid(SpawnedActor))
	{
		return;
	}

	SpawnedActor->SetActorEnableCollision(false);

	ActorOwnership = EPRActorOwnership::Owned;
	CurrentRotation = InitialRotation;

	RegisterPreviewActor(SpawnedActor);
}

void UPRActorPreviewPanel::SetPreviewActorByInstance(AActor* Actor, EPRActorOwnership Ownership)
{
	if (!IsValid(Actor))
	{
		return;
	}

	ClearPreview();

	if (!SceneCaptureComponent)
	{
		return;
	}

	ActorOwnership = Ownership;

	if (Ownership == EPRActorOwnership::External)
	{
		ApplyInstanceSettings(Actor);
	}
	else
	{
		Actor->SetActorLocation(PreviewWorldLocation);
		Actor->SetActorRotation(InitialRotation);
		Actor->SetActorEnableCollision(false);
		CurrentRotation = InitialRotation;
	}

	RegisterPreviewActor(Actor);
}

void UPRActorPreviewPanel::ClearPreview()
{
	UnregisterPreviewActor();
	CurrentRotation = InitialRotation;
}

void UPRActorPreviewPanel::SetRotation(FRotator NewRotation)
{
	CurrentRotation = NewRotation;
	ApplyActorRotation();
}

void UPRActorPreviewPanel::AddRotation(FRotator DeltaRotation)
{
	CurrentRotation += DeltaRotation;
	ApplyActorRotation();
}

void UPRActorPreviewPanel::ResetRotation()
{
	SetRotation(InitialRotation);
}

void UPRActorPreviewPanel::SetCameraDistance(float Distance)
{
	CurrentCameraDistance = FMath::Clamp(Distance, CameraSettings.MinDistance, CameraSettings.MaxDistance);
	UpdateCameraTransform();
}

void UPRActorPreviewPanel::SetAxisEnabled(EPRRotationAxis Axis, bool bEnabled)
{
	GetAxisSettingsRef(Axis).bEnabled = bEnabled;
}

void UPRActorPreviewPanel::SetAxisSettings(EPRRotationAxis Axis, const FPRAxisSettings& Settings)
{
	GetAxisSettingsRef(Axis) = Settings;
}

FPRAxisSettings UPRActorPreviewPanel::GetAxisSettings(EPRRotationAxis Axis) const
{
	switch (Axis)
	{
	case EPRRotationAxis::Pitch: return PitchSettings;
	case EPRRotationAxis::Yaw: return YawSettings;
	case EPRRotationAxis::Roll: return RollSettings;
	default: return YawSettings;
	}
}

void UPRActorPreviewPanel::SetAutoRotate(bool bEnabled, float Speed, EPRRotationAxis Axis)
{
	AutoRotateSettings.bEnabled = bEnabled;
	AutoRotateSettings.Speed = Speed;
	AutoRotateSettings.Axis = Axis;
}

void UPRActorPreviewPanel::SetInstanceSettings(const FPRInstanceSettings& Settings)
{
	InstanceSettings = Settings;
}

void UPRActorPreviewPanel::OnPreviewActorSet_Implementation(AActor* Actor)
{
}

void UPRActorPreviewPanel::OnPreviewActorCleared_Implementation(AActor* Actor)
{
}
