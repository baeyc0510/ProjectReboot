// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/TextureRenderTarget2D.h"

#include "PRActorPreviewTypes.generated.h"

UENUM(BlueprintType)
enum class EPRRotationAxis : uint8
{
    Pitch,
    Yaw,
    Roll
};

UENUM(BlueprintType)
enum class EPRDragInput : uint8
{
    None,
    DragX,
    DragY
};

// 프리뷰 액터 소유권
UENUM(BlueprintType)
enum class EPRActorOwnership : uint8
{
    Owned,      // 패널이 소유 (ClearPreview 시 Destroy)
    External    // 외부 소유 (ClearPreview 시 Destroy 안함)
};

USTRUCT(BlueprintType)
struct FPRAxisSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"))
    EPRDragInput DragInput = EPRDragInput::DragX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled", ClampMin = "0.01", ClampMax = "10.0", UIMin = "0.1", UIMax = "2.0"))
    float Sensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"))
    bool bInvert = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"))
    bool bClampAngle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled && bClampAngle", ClampMin = "-180.0", ClampMax = "180.0"))
    float MinAngle = -89.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled && bClampAngle", ClampMin = "-180.0", ClampMax = "180.0"))
    float MaxAngle = 89.f;
};

USTRUCT(BlueprintType)
struct FPRRenderSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "64", ClampMax = "4096"))
    FIntPoint Resolution = FIntPoint(512, 512);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ETextureRenderTargetFormat> Format = RTF_RGBA8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ClearColor =  FLinearColor(0.05f, 0.05f, 0.05f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseSRGB = true;
};

USTRUCT(BlueprintType)
struct FPRCameraSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", UIMin = "10.0", UIMax = "1000.0"))
    float Distance = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", UIMin = "10.0", UIMax = "500.0"))
    float MinDistance = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", UIMin = "50.0", UIMax = "2000.0"))
    float MaxDistance = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Angle = FRotator(-15.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Offset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "5.0", ClampMax = "170.0", UIMin = "15.0", UIMax = "120.0"))
    float FOV = 30.f;
};

USTRUCT(BlueprintType)
struct FPRZoomSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled", ClampMin = "0.1", ClampMax = "100.0", UIMin = "1.0", UIMax = "50.0"))
    float Sensitivity = 10.f;
};

USTRUCT(BlueprintType)
struct FPRAutoRotateSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled", ClampMin = "-360.0", ClampMax = "360.0", UIMin = "-180.0", UIMax = "180.0"))
    float Speed = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"))
    EPRRotationAxis Axis = EPRRotationAxis::Yaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnabled"))
    bool bPauseOnDrag = true;
};

// 프리뷰 조명 설정
USTRUCT(BlueprintType)
struct FPRLightSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator(-45.f, -45.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMax = "20.0"))
    float Intensity = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LightColor = FLinearColor::White;
};

// 인스턴스 설정 옵션
USTRUCT(BlueprintType)
struct FPRInstanceSettings
{
    GENERATED_BODY()

    // 프리뷰 위치로 이동할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRelocateToPreviewLocation = true;

    // 충돌 비활성화 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDisableCollision = true;

    // 액터 숨김 처리 (게임 월드에서)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHideInGame = true;
};