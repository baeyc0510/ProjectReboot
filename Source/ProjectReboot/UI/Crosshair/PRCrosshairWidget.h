// PRCrosshairWidget.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PRCrosshairWidget.generated.h"

class UPRCrosshairStyle;
class UPRCrosshairViewModel;
class UCanvasPanel;
class UImage;
class UTextBlock;

/**
 * 크로스헤어 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRCrosshairWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /*~ UUserWidget Interface ~*/
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    // 스타일 적용 (BP DesignTime에서도 활용 가능)
    UFUNCTION(BlueprintCallable)
    void ApplyStyle(UPRCrosshairStyle* Style);
    
    /*~ ViewModel Event Handlers ~*/
    UFUNCTION()
    void HandleSpreadChanged(float NewSpread);

    UFUNCTION()
    void HandleADSAlphaChanged(float NewAlpha);

    UFUNCTION()
    void HandleCanFireChanged(bool bCanFire);

    UFUNCTION()
    void HandleAmmoChanged(int32 Current, int32 Max);

    UFUNCTION()
    void HandleVisibilityChanged(bool bVisible);

    UFUNCTION()
    void HandleTargetingEnemyChanged(bool bTargeting);

    UFUNCTION()
    void HandleCrosshairTagChanged(const FGameplayTag& NewTag);

protected:
    // 탄약 텍스트 (선택적)
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AmmoText;

    // 크로스헤어 요소들
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CrosshairTop;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CrosshairBottom;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CrosshairLeft;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CrosshairRight;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CrosshairCenter;

private:
    /*~ Internal ~*/
    void BindViewModel();
    void UnbindViewModel();
    void ApplyInitialState();

    // 위치 업데이트
    void UpdateCrosshairPositions(float Spread);

    // 색상 업데이트
    void UpdateCrosshairColor();

    // 요소 색상 설정
    void SetCrosshairElementsColor(const FLinearColor& Color);

    // 요소 배열 반환
    TArray<UImage*> GetCrosshairElements() const;

private:
    UPROPERTY()
    TObjectPtr<UPRCrosshairViewModel> ViewModel;

    UPROPERTY()
    TObjectPtr<UPRCrosshairStyle> CurrentStyle;

    bool bIsTargetingEnemy = false;
    bool bCanFire = true;
    bool bHideOnADS = false;
};