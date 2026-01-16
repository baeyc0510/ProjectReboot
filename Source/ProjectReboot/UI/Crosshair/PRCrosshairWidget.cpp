// PRCrosshairWidget.cpp
#include "PRCrosshairWidget.h"

#include "PRCrosshairViewModel.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ProjectReboot/Crosshair/PRCrosshairConfig.h"
#include "ProjectReboot/Crosshair/PRCrosshairStyle.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

void UPRCrosshairWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindViewModel();
}

void UPRCrosshairWidget::NativeDestruct()
{
    UnbindViewModel();
    Super::NativeDestruct();
}

void UPRCrosshairWidget::BindViewModel()
{
    ULocalPlayer* LP = GetOwningLocalPlayer();
    if (!LP)
    {
        return;
    }

    UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
    if (!VMS)
    {
        return;
    }

    ViewModel = VMS->GetGlobalViewModel<UPRCrosshairViewModel>();
    if (!ViewModel)
    {
        return;
    }

    // 델리게이트 바인딩
    ViewModel->OnSpreadChanged.AddDynamic(this, &UPRCrosshairWidget::HandleSpreadChanged);
    ViewModel->OnADSAlphaChanged.AddDynamic(this, &UPRCrosshairWidget::HandleADSAlphaChanged);
    ViewModel->OnCanFireChanged.AddDynamic(this, &UPRCrosshairWidget::HandleCanFireChanged);
    ViewModel->OnAmmoChanged.AddDynamic(this, &UPRCrosshairWidget::HandleAmmoChanged);
    ViewModel->OnVisibilityChanged.AddDynamic(this, &UPRCrosshairWidget::HandleVisibilityChanged);
    ViewModel->OnTargetingEnemyChanged.AddDynamic(this, &UPRCrosshairWidget::HandleTargetingEnemyChanged);
    ViewModel->OnCrosshairTagChanged.AddDynamic(this, &UPRCrosshairWidget::HandleCrosshairTagChanged);

    ApplyInitialState();
}

void UPRCrosshairWidget::UnbindViewModel()
{
    if (!ViewModel)
    {
        return;
    }

    ViewModel->OnSpreadChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleSpreadChanged);
    ViewModel->OnADSAlphaChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleADSAlphaChanged);
    ViewModel->OnCanFireChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleCanFireChanged);
    ViewModel->OnAmmoChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleAmmoChanged);
    ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleVisibilityChanged);
    ViewModel->OnTargetingEnemyChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleTargetingEnemyChanged);
    ViewModel->OnCrosshairTagChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleCrosshairTagChanged);

    ViewModel = nullptr;
}

void UPRCrosshairWidget::ApplyInitialState()
{
    if (!ViewModel)
    {
        return;
    }

    const FPRCrosshairSetting& Setting = ViewModel->GetCurrentSetting();
    bHideOnADS = Setting.bHideOnADS;
    bCanFire = ViewModel->CanFire();
    bIsTargetingEnemy = ViewModel->IsTargetingEnemy();

    ApplyStyle(Setting.Style);
    HandleSpreadChanged(ViewModel->GetCurrentSpread());
    HandleADSAlphaChanged(ViewModel->GetADSAlpha());
    HandleVisibilityChanged(ViewModel->IsVisible());
    HandleAmmoChanged(ViewModel->GetCurrentAmmo(), ViewModel->GetMaxAmmo());
}

void UPRCrosshairWidget::HandleSpreadChanged(float NewSpread)
{
    UpdateCrosshairPositions(NewSpread);
}

void UPRCrosshairWidget::HandleADSAlphaChanged(float NewAlpha)
{
    if (bHideOnADS)
    {
        const float Opacity = FMath::Lerp(1.0f, 0.0f, NewAlpha);
        SetRenderOpacity(Opacity);
    }
}

void UPRCrosshairWidget::HandleCanFireChanged(bool bNewCanFire)
{
    bCanFire = bNewCanFire;
    UpdateCrosshairColor();
}

void UPRCrosshairWidget::HandleAmmoChanged(int32 Current, int32 Max)
{
    if (AmmoText)
    {
        AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max)));
    }
}

void UPRCrosshairWidget::HandleVisibilityChanged(bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPRCrosshairWidget::HandleTargetingEnemyChanged(bool bTargeting)
{
    bIsTargetingEnemy = bTargeting;
    UpdateCrosshairColor();
}

void UPRCrosshairWidget::HandleCrosshairTagChanged(const FGameplayTag& NewTag)
{
    if (!ViewModel)
    {
        return;
    }

    const FPRCrosshairSetting& Setting = ViewModel->GetCurrentSetting();
    bHideOnADS = Setting.bHideOnADS;

    // 스타일 적용
    ApplyStyle(Setting.Style);

    // ADS 투명도 재적용
    if (bHideOnADS)
    {
        HandleADSAlphaChanged(ViewModel->GetADSAlpha());
    }
    else
    {
        SetRenderOpacity(1.0f);
    }
}
void UPRCrosshairWidget::ApplyStyle(UPRCrosshairStyle* Style)
{
    CurrentStyle = Style;

    if (!Style)
    {
        if (CrosshairTop) CrosshairTop->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairBottom) CrosshairBottom->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairLeft) CrosshairLeft->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairRight) CrosshairRight->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairCenter) CrosshairCenter->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    // Top
    if (CrosshairTop)
    {
        if (Style->TopTexture)
        {
            CrosshairTop->SetBrushFromTexture(Style->TopTexture);
            CrosshairTop->SetDesiredSizeOverride(Style->ElementSize);
            CrosshairTop->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairTop->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Bottom
    if (CrosshairBottom)
    {
        if (Style->BottomTexture)
        {
            CrosshairBottom->SetBrushFromTexture(Style->BottomTexture);
            CrosshairBottom->SetDesiredSizeOverride(Style->ElementSize);
            CrosshairBottom->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairBottom->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Left
    if (CrosshairLeft)
    {
        if (Style->LeftTexture)
        {
            CrosshairLeft->SetBrushFromTexture(Style->LeftTexture);
            CrosshairLeft->SetDesiredSizeOverride(FVector2D(Style->ElementSize.Y, Style->ElementSize.X));
            CrosshairLeft->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairLeft->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Right
    if (CrosshairRight)
    {
        if (Style->RightTexture)
        {
            CrosshairRight->SetBrushFromTexture(Style->RightTexture);
            CrosshairRight->SetDesiredSizeOverride(FVector2D(Style->ElementSize.Y, Style->ElementSize.X));
            CrosshairRight->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairRight->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Center
    if (CrosshairCenter)
    {
        if (Style->bShowCenter && Style->CenterTexture)
        {
            CrosshairCenter->SetBrushFromTexture(Style->CenterTexture);
            CrosshairCenter->SetDesiredSizeOverride(Style->CenterSize);
            CrosshairCenter->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairCenter->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    UpdateCrosshairColor();
}

void UPRCrosshairWidget::UpdateCrosshairPositions(float Spread)
{
    if (CrosshairTop)
    {
        if (UCanvasPanelSlot* PanelSlot =  Cast<UCanvasPanelSlot>(CrosshairTop->Slot))
        {
            PanelSlot->SetPosition(FVector2D(0.0f, -Spread));
        }
    }

    if (CrosshairBottom)
    {
        if (UCanvasPanelSlot* PanelSlot =  Cast<UCanvasPanelSlot>(CrosshairBottom->Slot))
        {
            PanelSlot->SetPosition(FVector2D(0.0f, Spread));
        }
    }

    if (CrosshairLeft)
    {
        if (UCanvasPanelSlot* PanelSlot =  Cast<UCanvasPanelSlot>(CrosshairLeft->Slot))
        {
            PanelSlot->SetPosition(FVector2D(-Spread, 0.0f));
        }
    }

    if (CrosshairRight)
    {
        if (UCanvasPanelSlot* PanelSlot =  Cast<UCanvasPanelSlot>(CrosshairRight->Slot))
        {
            PanelSlot->SetPosition(FVector2D(Spread, 0.0f));
        }
    }
}

void UPRCrosshairWidget::UpdateCrosshairColor()
{
    if (!CurrentStyle)
    {
        return;
    }

    FLinearColor TargetColor = CurrentStyle->DefaultColor;

    if (!bCanFire)
    {
        TargetColor = CurrentStyle->CannotFireColor;
    }
    else if (bIsTargetingEnemy)
    {
        TargetColor = CurrentStyle->EnemyTargetColor;
    }

    SetCrosshairElementsColor(TargetColor);
}

void UPRCrosshairWidget::SetCrosshairElementsColor(const FLinearColor& Color)
{
    for (UImage* Element : GetCrosshairElements())
    {
        if (Element)
        {
            Element->SetColorAndOpacity(Color);
        }
    }
}

TArray<UImage*> UPRCrosshairWidget::GetCrosshairElements() const
{
    return { CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight, CrosshairCenter };
}