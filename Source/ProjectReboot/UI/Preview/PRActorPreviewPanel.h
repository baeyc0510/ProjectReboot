// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRActorPreviewTypes.h"
#include "Blueprint/UserWidget.h"
#include "PRActorPreviewPanel.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRActorPreviewPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    UPRActorPreviewPanel(const FObjectInitializer& ObjectInitializer);

    /*~ UPRActorPreviewPanel Interfaces ~*/
    
    // ActorClass로부터 액터를 직접 소환하여 PreviewTarget으로 지정
    UFUNCTION(BlueprintCallable, Category = "ActorPreview")
    void SetPreviewActorByClass(TSubclassOf<AActor> ActorClass);
    
    // 이미 Spawn된 Actor를 PreviewTarget으로 지정
    UFUNCTION(BlueprintCallable, Category = "ActorPreview")
    void SetPreviewActorByInstance(AActor* Actor, EPRActorOwnership Ownership = EPRActorOwnership::External);

    UFUNCTION(BlueprintCallable, Category = "ActorPreview")
    void ClearPreview();

    UFUNCTION(BlueprintPure, Category = "ActorPreview")
    AActor* GetPreviewActor() const { return PreviewActor; }

    UFUNCTION(BlueprintPure, Category = "ActorPreview")
    bool HasPreviewActor() const { return PreviewActor != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Rotation")
    void SetRotation(FRotator NewRotation);

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Rotation")
    void AddRotation(FRotator DeltaRotation);

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Rotation")
    void ResetRotation();

    UFUNCTION(BlueprintPure, Category = "ActorPreview|Rotation")
    FRotator GetRotation() const { return CurrentRotation; }

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintPure, Category = "ActorPreview|Camera")
    float GetCameraDistance() const { return CurrentCameraDistance; }

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Axis")
    void SetAxisEnabled(EPRRotationAxis Axis, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Axis")
    void SetAxisSettings(EPRRotationAxis Axis, const FPRAxisSettings& Settings);

    UFUNCTION(BlueprintPure, Category = "ActorPreview|Axis")
    FPRAxisSettings GetAxisSettings(EPRRotationAxis Axis) const;

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|AutoRotate")
    void SetAutoRotate(bool bEnabled, float Speed = 30.f, EPRRotationAxis Axis = EPRRotationAxis::Yaw);

    UFUNCTION(BlueprintCallable, Category = "ActorPreview|Instance")
    void SetInstanceSettings(const FPRInstanceSettings& Settings);

    UFUNCTION(BlueprintNativeEvent, Category = "ActorPreview")
    void OnPreviewActorSet(AActor* Actor);
    
    UFUNCTION(BlueprintNativeEvent, Category = "ActorPreview")
    void OnPreviewActorCleared(AActor* Actor);
    
protected:
    /*~ UUserWidget Interfaces ~*/
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /*~ UPRActorPreviewPanel Interfaces ~*/
    virtual UTextureRenderTarget2D* GetOrCreateRenderTarget();
    virtual void ConfigureSceneCapture(USceneCaptureComponent2D* CaptureComponent);
    virtual void ApplyToImage(UTexture* Texture);

private:
    void InitializePreviewScene();
    void DestroyPreviewScene();
    void UpdateCameraTransform();
    void ApplyActorRotation();
    float ProcessAxisInput(const FPRAxisSettings& Settings, const FVector2D& Delta, float CurrentValue);

    void RegisterPreviewActor(AActor* Actor);
    void UnregisterPreviewActor();
    void ApplyInstanceSettings(AActor* Actor);
    void RestoreInstanceSettings(AActor* Actor);

    FPRAxisSettings& GetAxisSettingsRef(EPRRotationAxis Axis);
    double& GetRotationAxisRef(EPRRotationAxis Axis);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_Preview;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Settings")
    FPRRenderSettings RenderSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Settings")
    FPRCameraSettings CameraSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Settings")
    FPRZoomSettings ZoomSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Settings")
    FPRAutoRotateSettings AutoRotateSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Settings")
    FPRInstanceSettings InstanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Rotation")
    FPRAxisSettings PitchSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Rotation")
    FPRAxisSettings YawSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Rotation")
    FPRAxisSettings RollSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Rotation")
    FRotator InitialRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Scene")
    FVector PreviewWorldLocation = FVector(10000.f, 10000.f, 10000.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Advanced", AdvancedDisplay)
    TObjectPtr<UTextureRenderTarget2D> RenderTargetOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorPreview|Advanced", AdvancedDisplay)
    TObjectPtr<UMaterialInterface> MaterialOverride;

private:
    UPROPERTY()
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;

    UPROPERTY()
    TObjectPtr<AActor> SceneCaptureActor;

    UPROPERTY()
    TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

    UPROPERTY()
    TObjectPtr<AActor> PreviewActor;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    struct FActorRestoreData
    {
        FVector OriginalLocation;
        FRotator OriginalRotation;
        bool bOriginalCollisionEnabled;
        bool bOriginalHiddenInGame;
        bool bIsValid = false;
    };
    FActorRestoreData RestoreData;

    EPRActorOwnership ActorOwnership = EPRActorOwnership::Owned;
    FRotator CurrentRotation = FRotator::ZeroRotator;
    float CurrentCameraDistance = 0.f;
    FVector2D LastMousePosition = FVector2D::ZeroVector;
    bool bIsDragging = false;
    bool bIsInitialized = false;
};